#include "anealing.h"

namespace anealing
{
    class Schedule_abstr
    {
    protected:
        size_t cpu_cnt_;
        std::vector<std::vector<size_t>> schedule_;
        std::unordered_map<size_t, size_t> task_time_;

    public:
        virtual size_t get_time() = 0;
        virtual size_t get_proc_num() const = 0;
        virtual std::vector<size_t> get_proc_tasks(size_t proc) const = 0;
        virtual std::vector<std::vector<size_t>> get_schedule() const = 0;
        virtual void set_schedule(std::vector<std::vector<size_t>> &schedule) = 0;
    };

    class Schedule : public Schedule_abstr
    {
    public:
        Schedule(size_t cpu_cnt, const std::unordered_map<size_t, size_t> &task_time)
        {
            cpu_cnt_ = cpu_cnt;
            schedule_.resize(cpu_cnt_);
            task_time_ = task_time;
        }
        Schedule() {}

        size_t get_time()
        {
            size_t time = 0;

            for (auto &cpu : schedule_)
            {
                size_t cur_time = 0;

                for (auto task : cpu)
                {
                    cur_time += task_time_[task];
                }

                time = std::max(time, cur_time);
            }

            return time;
        }

        size_t get_proc_num() const
        {
            return cpu_cnt_;
        }

        std::vector<size_t> get_proc_tasks(size_t proc) const
        {
            return schedule_[proc];
        }

        std::vector<std::vector<size_t>> get_schedule() const
        {
            return schedule_;
        }

        void set_schedule(std::vector<std::vector<size_t>> &schedule)
        {
            schedule_ = schedule;
        }
    };

    template <class Schedule_>
    class Mutation_abstr
    {
    public:
        virtual Schedule_ transform(const Schedule_ &cur_schedule) = 0;
    };

    template <class T>
    class Mutation : public Mutation_abstr<T>
    {
    public:
        T transform(const T &cur_schedule)
        {
            // mutation operation - move task from one cpu to another
            if (cur_schedule.get_proc_num() < 2)
            {
                return cur_schedule;
            }


            size_t proc = rand() % cur_schedule.get_proc_num();
            auto cur_cpu = cur_schedule.get_proc_tasks(proc);

            while (!cur_cpu.size())
            {
                proc = rand() % cur_schedule.get_proc_num();
                cur_cpu = cur_schedule.get_proc_tasks(proc);
            }

            auto new_schedule = cur_schedule.get_schedule();
            size_t pos_to_move = rand() % cur_cpu.size();
            size_t task = new_schedule[proc][pos_to_move];

            new_schedule[proc].erase(new_schedule[proc].begin() + pos_to_move);

            size_t new_proc = rand() % cur_schedule.get_proc_num();

            while (new_proc == proc)
            {
                new_proc = rand() % cur_schedule.get_proc_num();
            }

            new_schedule[new_proc].push_back(task);

            T next_schedule = cur_schedule;

            next_schedule.set_schedule(new_schedule);

            return next_schedule;
        }
    };

    class Temp_change_abstr
    {
    protected:
        double temperature_;
        double start_temperature_;

    public:
        virtual double temprature_step(size_t iteration) = 0;

        double get_temprature()
        {
            return temperature_;
        }
    };

    class Boltzman : public Temp_change_abstr
    {
    public:
        Boltzman(double start_temperature = 0)
        {
            temperature_ = start_temperature_ = start_temperature;
        }

        double temprature_step(size_t iteration)
        {
            temperature_ = (double)start_temperature_ / std::log(1 + (iteration + 1));
            return temperature_;
        }
    };

    class Cauchy : public Temp_change_abstr
    {
    public:
        Cauchy(double start_temperature = 0)
        {
            temperature_ = start_temperature_ = start_temperature;
        }

        double temprature_step(size_t iteration)
        {
            temperature_ = (double)start_temperature_ / (1 + iteration);
            return temperature_;
        }
    };

    class LogDiv : public Temp_change_abstr
    {
    public:
        LogDiv(double start_temperature = 0)
        {
            temperature_ = start_temperature_ = start_temperature;
        }

        double temprature_step(size_t iteration)
        {
            temperature_ = (double)start_temperature_ * std::log(1 + (iteration + 1)) / (1 + (iteration + 1));
            return temperature_;
        }
    };

    class Temperature
    {
        size_t temp_law_;
        Boltzman boltzman_law;
        Cauchy cauchy_law;
        LogDiv logdiv_law;

    public:
        Temperature(size_t temp_law, double start_temp) : temp_law_(temp_law)
        {
            if (temp_law_ == 1)
            {
                boltzman_law = Boltzman(start_temp);
            }
            else if (temp_law_ == 2)
            {
                cauchy_law = Cauchy(start_temp);
            }
            else
            {
                logdiv_law = LogDiv(start_temp);
            }
        }

        double temprature_step(size_t iteration)
        {
            if (temp_law_ == 1)
            {
                return boltzman_law.temprature_step(iteration);
            }
            else if (temp_law_ == 2)
            {
                return cauchy_law.temprature_step(iteration);
            }
            else
            {
                return logdiv_law.temprature_step(iteration);
            }
        }
    };

    template <class T>
    class Anealing
    {
        double start_temp_;
        size_t temp_law_;
        size_t it_without_change_;
        size_t temp_it_;
        T start_schedule_;

        size_t best_time_ = UINT32_MAX;
        T best_schedule_;

    public:
        Anealing(double start_temp, size_t temp_law, size_t it_without_change, size_t temp_it, T &start_schedule) : start_temp_(start_temp), temp_law_(temp_law), it_without_change_(it_without_change), temp_it_(temp_it), start_schedule_(start_schedule) {}

        T mainloop()
        {
            if (start_schedule_.get_proc_num() < 2)
            {
                return start_schedule_;
            }

            int cur_it_without_change{0};
            int iteration{0};

            Temperature temp_change(temp_law_, start_temp_);
            T cur_schedule = start_schedule_;
            size_t cur_time = cur_schedule.get_time();
            Mutation<T> mutator;

            while (cur_it_without_change < it_without_change_)
            {
                ++iteration;

                double cur_temp = temp_change.temprature_step(iteration);

                for (int it = 0; it < temp_it_; ++it)
                {
                    T new_schedule = mutator.transform(cur_schedule);

                    size_t new_time = new_schedule.get_time();

                    if (new_time < best_time_)
                    {
                        best_time_ = new_time;
                        best_schedule_ = new_schedule;
                        cur_schedule = new_schedule;
                        cur_time = new_time;
                        cur_it_without_change = 0;
                    }
                    else
                    {
                        cur_it_without_change++;

                        double prob = (double)rand() / RAND_MAX;

                        if (prob <= exp((double)(cur_time - new_time) / cur_temp))
                        {
                            cur_time = new_time;
                            cur_schedule = new_schedule;
                        }
                    }
                }
            }

            return best_schedule_;
        }
    };
}

int main()
{
    for (int proc_num = 1; proc_num < 100; ++proc_num)
    {
        std::unordered_map<size_t, size_t> proc;

        for (int i = 0; i < proc_num; ++i)
        {
            proc[i] = rand() % 100 + 1;
        }

        for (int cpu_num = 1; cpu_num < 100; ++cpu_num)
        {
            anealing::Schedule sch(cpu_num, proc);
            std::vector<std::vector<size_t>> start_sch(cpu_num);

            for (int i = 0; i < proc_num; ++i)
            {
                start_sch[0].push_back(i);
            }

            sch.set_schedule(start_sch);
            
            std::cout << "num_proc : " << proc_num << " cpu_num : " << cpu_num << "\n";

            anealing::Anealing<anealing::Schedule> a(100, 1, 100, 5, sch);


            auto best = a.mainloop();

            auto tmp = best.get_schedule();

            int i = 1;

            for (auto &cpu : tmp)
            {
                std::cout << "CPU" << i << " : ";
                for (auto it : cpu)
                {
                    std::cout << it << ' ';
                }
                std::cout << "\n";
                i++;
            }

            std::cout << "\n";
        }
    }
}