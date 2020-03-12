////
//// Created by Mark Plagge on 2/27/20.
////
//
//#include <catch2/catch.hpp>
//#include <fcfs_logic_system.h>
//#include <vector>
//#include <random>
//#include <cstdlib>
//#include <iostream>
//#include <simclist.h>
//#ifdef __cplusplus
//extern "C"
//{
//#endif
//#include "../src/simulated_process_queue.h"
//#ifdef __cplusplus
//}
//#endif
///** Process List Generator for catch2
// *
// */
//
//
//class ProcListGenerator : public Catch::Generators::IGenerator<std::vector<simulated_process *>> {
//    std::default_random_engine generator;
//    std::vector<simulated_process *> current_list;
//    std::vector<std::vector<simulated_process *>> list_of_proc_lists;
//    int vec_pos;
//    int ttl_lists;
//
//public:
//    ProcListGenerator(int min_q_size, int max_q_size, int min_time = 1, int max_time = 65535,
//                      int min_core = 1, int max_core = 4096, int pid_min = 1, int pid_max = 65535) {
//        this->vec_pos = 0;
//        this->ttl_lists = 0;
//
//        // not running the cartesian product of time, core, and pids... just going to randomly select them.
//        std::uniform_int_distribution<int> time_dist(min_time, max_time);
//        std::uniform_int_distribution<int> core_dist(min_core, max_time);
//        std::uniform_int_distribution<int> pid_dist(pid_min, pid_max);
//        auto cores_needed = std::bind(core_dist, generator);
//        auto time_needed = std::bind(time_dist, generator);
//        auto pid = std::bind(pid_dist, generator);
//
//        for (int current_q_size = min_q_size; current_q_size <= max_q_size; ++current_q_size) {
//            std::vector<simulated_process *> process_list;
//            for (int pd = 0; pd < min_q_size; pd++) {
//                int p_cores_needed = cores_needed();
//                int p_time_needed = time_needed();
//                int p_pid = pid();
//                simulated_process *p = new_simulated_process_cores_time(p_cores_needed, p_time_needed, p_pid);
//                process_list.push_back(p);
//            }
//            this->list_of_proc_lists.push_back(process_list);
//
//        }
//        this->ttl_lists = this->list_of_proc_lists.size();
//
//        static_cast<void> (next());
//
//    }
//
//    std::vector<simulated_process *> const &get() const override;
//
//    bool next() override {
//
//        for (auto p : this->current_list) {
//            free(p);
//        }
//        current_list = list_of_proc_lists[vec_pos];
//        vec_pos++;
//        if (vec_pos < ttl_lists) {
//            return true;
//        }
//        return false;
//
//    }
//
//};
//
//std::vector<simulated_process *> const &ProcListGenerator::get() const {
//    return this->current_list;
//}
//
//Catch::Generators::GeneratorWrapper<std::vector<simulated_process *>> process_list_gen(int min_q_size, int max_q_size) {
//    return Catch::Generators::GeneratorWrapper<std::vector<simulated_process *>>
//            (std::unique_ptr<Catch::Generators::IGenerator<std::vector<simulated_process *>>>(
//                    new ProcListGenerator(min_q_size, max_q_size)));
//}
//////////////////////////////////////////////////////////////////////
//TEST_CASE("Process logic works properly", "[simulated_process]") {
//
//    int max_time = 128;
//    std::default_random_engine genz;
//    std::uniform_int_distribution<int> distribution(32, max_time);
//    std::uniform_int_distribution<int> core_dist(16, 4096);
//    std::uniform_int_distribution<int> pid_dist(1, 8192);
//    auto cores_needed = std::bind(core_dist, genz);
//    auto time_needed = std::bind(distribution, genz);
//    auto pid = std::bind(pid_dist, genz);
//    int p_cores_needed = cores_needed();
//    int p_time_needed = time_needed();
//    int p_pid = pid();
//
//
//
//
//    SECTION("Processes change state properly") {
//        simulated_process *p = new_simulated_process_cores_time(p_cores_needed, p_time_needed, p_pid);
//        REQUIRE(p->current_state == WAITING);
//        p->current_state = RUNNING;
//
//        for (int i = 0; i < p_time_needed; ++i) {
//            REQUIRE(p->current_state == RUNNING);
//            simulated_process_tick(p);
//        }
//        std::cout << p->total_run_time << "|" << p_time_needed << "\n";
//        REQUIRE(p->current_state == COMPLETE);
//
//
//    }SECTION("Processes log wait_time and run_time properly") {
//        simulated_process *p = new_simulated_process_cores_time(p_cores_needed, p_time_needed, p_pid);
//        //wait for some time:
//        int p_wait_time = time_needed();
//        int current_tick = 0;
//        for (; current_tick < p_wait_time; current_tick++) {
//            simulated_process_tick(p);
//        }
//        p->current_state = RUNNING;
//        for (int i = 0; i < p_time_needed; ++i) {
//            simulated_process_tick(p);
//        }
//        REQUIRE(p->total_wait_time == p_wait_time);
//        REQUIRE(p->total_run_time == p_time_needed);
//
//    }
//
//    SECTION("sim process log wait times properly"){
//        auto num_waits = GENERATE(range(1,1000));
//        simulated_process *p = new_simulated_process_cores_time(p_cores_needed, p_time_needed, p_pid);
//        for(int i =0; i < num_waits; i ++){
//            simulated_process_tick(p);
//        }
//        REQUIRE(p->total_wait_time == num_waits);
//
//    }
//}
//
//
//
//TEST_CASE("Process queue SIMCLIST impl tests", "[simulated_process][queue_list]") {
//    auto process_list = GENERATE(take(5, process_list_gen(1, 6000)));
//    proc_q_list *q = create_queue_list();
//
//    SECTION("Enqueue / QLEN test") {
//        //tests if a queue can enqueue various numbers of processes
//        //and makes sure that QLEN works.
//        int num_to_queue = process_list.size();
//        int num_in_q = 0;
//        for (auto p : process_list) {
//
//            proc_q_list_enq(q, p);
//            REQUIRE(proc_q_list_size(q) == ++num_in_q);
//        }
//
//        REQUIRE(proc_q_list_size(q) == num_to_queue);
//    }
//    SECTION("Queue dequeue test") {
//        std::vector<simulated_process *> dequeued_procs;
//        int num_in_q = process_list.size();
//        //enqueue
//        for (auto p : process_list) {
//            proc_q_list_enq(q, p);
//        }
//
//        int num_dequeued = 0;
//        simulated_process *p = proc_q_list_deq(q);
//        if (num_in_q == 0) {
//            REQUIRE(p == NULL);
//        } else {
//            num_dequeued = 1;
//            //null check loop test
//
//            while (p != NULL) {
//                dequeued_procs.push_back(p);
//                p = proc_q_list_deq(q);
//                num_dequeued++;
//            }
//            REQUIRE(dequeued_procs.size() == num_in_q);
//        }
//
//
//    }
//
//
//    SECTION("Queue process ticks apply wait-time"){
//        auto num_ticks = GENERATE(range(1,50000));
//
//        for (auto p : process_list) {
//            p->total_wait_time = 0;
//            p->total_run_time = 0;
//            proc_q_list_enq(q, p);
//        }
//        for (int t = 0; t < num_ticks; t ++){
//            proc_q_list_tick(q);
//
//        }
//        //we ticked num_ticks. Each process should have advanced
//        for(int i = 0; i < process_list.size(); i ++){
//            simulated_process *p =(simulated_process *) list_get_at(q->queue_list,i);
//            if(p->total_wait_time != num_ticks) {
//                REQUIRE(p->total_wait_time == num_ticks);
//            }
//        }
//
//    }
//    SECTION("Queue process ticks apply run-time"){
//        auto num_ticks = GENERATE(range(1,50000));
//        for (auto p : process_list) {
//            p->total_wait_time = 0;
//            p->total_run_time = 0;
//            p->current_state = RUNNING;
//            proc_q_list_enq(q, p);
//        }
//        for (int t = 0; t < num_ticks; t ++){
//            proc_q_list_tick(q);
//        }
//        for(int i = 0; i < process_list.size(); i ++){
//            simulated_process *p =(simulated_process *) list_get_at(q->queue_list,i);
//            REQUIRE(p->total_wait_time == 0);
//            int max_runtime = p->needed_run_time;
//            bool runtime_check = p->total_run_time == num_ticks;
//            runtime_check = runtime_check || (p->total_run_time == max_runtime && p->current_state == COMPLETE);
//            if(not runtime_check){
//                CHECK(p->total_run_time == num_ticks);
//                CHECK(p->total_run_time == max_runtime);
//                CHECK(p->current_state == COMPLETE);
//            }
//            REQUIRE(runtime_check);
//
//
//        }
//
//    }
//}
//
//TEST_CASE("Simclist VS Custom") {
//    auto process_list = GENERATE(take(1, process_list_gen(10000, 11000)));
//    proc_queue *q = create_queue();
//    proc_q_list *ql = create_queue_list();
//
//    BENCHMARK("Custom Q-DQ Test") {
//
//
//        std::vector<simulated_process *> dequeued_procs;
//        int num_in_q = process_list.size();
//        //enqueue
//        for (auto p : process_list) {
//            proc_q_enqueue(q, p);
//        }
//
//        int num_dequeued = 0;
//        simulated_process *p = proc_q_dequeue(q);
//        if (num_in_q == 0) {
//            //REQUIRE(p == NULL);
//        } else {
//            num_dequeued = 1;
//            //null check loop test
//
//            while (p != NULL) {
//                dequeued_procs.push_back(p);
//                p = proc_q_dequeue(q);
//                num_dequeued++;
//            }
//
//        }
//    };
//
//    BENCHMARK("Queue dequeue test") {
//        std::vector<simulated_process *> dequeued_procs;
//        int num_in_q = process_list.size();
//        //enqueue
//        for (auto p : process_list) {
//            proc_q_list_enq(ql, p);
//        }
//
//        int num_dequeued = 0;
//        simulated_process *p = proc_q_list_deq(ql);
//        if (num_in_q == 0) {
//
//        } else {
//            num_dequeued = 1;
//            //null check loop test
//
//            while (p != NULL) {
//                dequeued_procs.push_back(p);
//                p = proc_q_list_deq(ql);
//                num_dequeued++;
//            }
//
//        }
//
//
//    };
//
//    BENCHMARK("Queue-List TICK benchmark"){
//        auto num_ticks = 50000;
//        for (auto p : process_list) {
//            p->total_wait_time = 0;
//            p->total_run_time = 0;
//            p->current_state = RUNNING;
//            proc_q_list_enq(ql, p);
//        }
//        for (int t = 0; t < num_ticks; t ++){
//            proc_q_list_tick(ql);
//        }
//
//    };
//    BENCHMARK("Queue-List TICK benchmark"){
//        auto num_ticks = 50000;
//        for (auto p : process_list) {
//             p->total_wait_time = 0;
//             p->total_run_time = 0;
//             p->current_state = RUNNING;
//             proc_q_enqueue(q, p);
//         }
//         for (int t = 0; t < num_ticks; t ++){
//             proc_q_tick(q);
//         }
//
//    };
//}
//
//
//TEST_CASE("process queue works properly", "[simulated_process][queue]") {
//
//    int max_time = 128;
//    int max_core = 4096;
//    int pid_max = 65535;
//
//    int min_time = 1;
//    int min_core = 1;
//    int pid_min = 1;
//
//
//    auto process_list = GENERATE(take(1000, process_list_gen(1, 5000)));
//    proc_queue *q = create_queue();
//
//
//
//
//    SECTION("Enqueue / QLEN test") {
//        //tests if a queue can enqueue various numbers of processes
//        //and makes sure that QLEN works.
//        int num_to_queue = process_list.size();
//        int num_in_q = 0;
//        for (auto p : process_list) {
//
//            proc_q_enqueue(q, p);
//            REQUIRE(proc_q_size(q) == ++num_in_q);
//        }
//
//        REQUIRE(proc_q_size(q) == num_to_queue);
//    }
//
//
//    SECTION("Queue dequeue test") {
//        std::vector<simulated_process *> dequeued_procs;
//        int num_in_q = process_list.size();
//        //enqueue
//        for (auto p : process_list) {
//            proc_q_enqueue(q, p);
//        }
//
//        int num_dequeued = 0;
//        simulated_process *p = proc_q_dequeue(q);
//        if (num_in_q == 0) {
//            REQUIRE(p == NULL);
//        } else {
//            num_dequeued = 1;
//            //null check loop test
//
//            while (p != NULL) {
//                dequeued_procs.push_back(p);
//                p = proc_q_dequeue(q);
//                num_dequeued++;
//            }
//            REQUIRE(dequeued_procs.size() == num_in_q);
//        }
//
//
//    }
//
//
//    SECTION("Queue process tick test") {
//        int num_in_q = process_list.size();
//        //enqueue
//        for (auto p : process_list) {
//            proc_q_enqueue(q, p);
//        }
//
//    }
//}