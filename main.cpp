#include "anealing.h"

// requres five cmd parametres output file, cpu number, task number, minimal task duration, maximal task duration

int main(int argc, char **argv)
{
    srand(time(0));

    anealing::test_generator("test", std::stoi(argv[2]), std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]));

    auto d = anealing::read_data("test");

    anealing::First_schedule<anealing::Schedule> sch_gen;
    anealing::Schedule start_schedule = sch_gen.generate_dummy_schedule(d.num_cpu, d.num_task, d.exec_time);

    std::cout << "Start time: " << start_schedule.get_time() << "\n";

    // args: start_temp, temp_law, it_without_change, temp_it, start_schedule
    anealing::Anealing<anealing::Schedule> main_proc(100, 1, 100, 5, start_schedule);

    auto begin = std::chrono::high_resolution_clock::now();

    anealing::Schedule res_sch = main_proc.mainloop();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();

    std::cout << "Result!\nTime per cpu: ";
    
    for (auto it : res_sch.time_per_cpu()){
        std::cout << it << " ";
    }
    std::cout << "\n";

    std::cout << "Result time: " << res_sch.get_time() << "\nComputation time: " << duration / 1e9 << "\n";

    /*std::vector<size_t> num_cpu = {2, 10, 100, 1000, 10000};
    std::vector<size_t> num_tasks = {10, 100, 1000, 10000, 100000, 300000, 600000, 900000, 1200000, 1500000};

    for (int i = 0; i < num_cpu.size(); ++i)
    {
        for (int j = 0; j < num_tasks.size(); ++j)
        {
            // anealing::test_generator("test", std::stoi(argv[2]), std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]));
            anealing::test_generator("test", num_cpu[i], num_tasks[j], 1, 100);

            auto d = anealing::read_data("test");
            long long t;
            int N = 3;

            for (int k = 0; k < N; ++k)
            {
                anealing::First_schedule<anealing::Schedule> sch_gen;
                anealing::Schedule start_schedule = sch_gen.generate_schedule(d.num_cpu, d.num_task, d.exec_time);

                //std::cout << "Start time: " << start_schedule.get_time() << "\n";

                // args: start_temp, temp_law, it_without_change, temp_it, start_schedule
                anealing::Anealing<anealing::Schedule> main_proc(1000, 3, 100, 5, start_schedule);

                auto begin = std::chrono::high_resolution_clock::now();

                anealing::Schedule res_sch = main_proc.mainloop();

                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();

                t += duration;
            }

            t /= N;

            std::cout << num_cpu[i] <<  ' ' << num_tasks[j] << ' ' << t / 1e9 << '\n';
            //std::cout << "Result time: " << res_sch.get_time() << "\nComputation time: " << duration / 1e9 << "\n";
        }
    }*/
}