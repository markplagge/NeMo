//
// Created by Mark Plagge on 3/11/20.
//

#include <catch2/catch.hpp>
#include <fcfs_logic_system.h>
#include <random>
#include "../lib/json.hpp"
using namespace neuro_os::sim_proc;
class ProcListGenerator : public Catch::Generators::IGenerator<std::vector<SimProcess<int>> >{
    std::default_random_engine generator;
    std::vector<SimProcess<int>> current_list;
    std::vector<std::vector<SimProcess<int>>> list_of_proc_lists;
    int vec_pos;
    int ttl_lists;
public:

    ProcListGenerator(int min_q_size, int max_q_size, int min_time = 1, int max_time = 65535,
                      int min_core = 1, int max_core = 4096, int pid_min = 1, int pid_max = 65535) {
        this->vec_pos = 0;
        this->ttl_lists = 0;

        // not running the cartesian product of time, core, and pids... just going to randomly select them.
        std::uniform_int_distribution<int> time_dist(min_time, max_time);
        std::uniform_int_distribution<int> core_dist(min_core, max_time);
        std::uniform_int_distribution<int> pid_dist(pid_min, pid_max);
        auto cores_needed = std::bind(core_dist, generator);
        auto time_needed = std::bind(time_dist, generator);
        auto pid = std::bind(pid_dist, generator);

        for (int current_q_size = min_q_size; current_q_size <= max_q_size; ++current_q_size) {
            std::vector<SimProcess<int>> process_list;
            for (int pd = 0; pd < min_q_size; pd++) {
                int p_cores_needed = cores_needed();
                int p_time_needed = time_needed();
                int p_pid = pid();
                SimProcess<int> p = SimProcess<int>(p_pid,p_cores_needed,p_time_needed,0.0,0);
                process_list.push_back(p);
            }
            this->list_of_proc_lists.push_back(process_list);

        }
        this->ttl_lists = this->list_of_proc_lists.size();

        static_cast<void> (next());

    }

    std::vector<SimProcess<int>> const &get() const override;

    bool next() override {

        for (auto p : this->current_list) {

        }
        current_list = list_of_proc_lists[vec_pos];
        vec_pos++;
        return vec_pos < ttl_lists;

    }

};

std::vector<SimProcess<int>> const &ProcListGenerator::get() const {
    return this->current_list;
}

Catch::Generators::GeneratorWrapper<std::vector<SimProcess<int>>> process_list_gen(int min_q_size, int max_q_size) {
    return Catch::Generators::GeneratorWrapper<std::vector<SimProcess<int>>>
            (std::unique_ptr<Catch::Generators::IGenerator<std::vector<SimProcess<int>>>>(
                    new ProcListGenerator(min_q_size, max_q_size)));
};

void check_p(SimProcess<int> a, SimProcess<int> b){
    REQUIRE(a.getTotalWaitTime() == b.getTotalWaitTime());
    REQUIRE(a.getCurrentState() == b.getCurrentState());
    REQUIRE(a.getTotalRunTime() == b.getTotalRunTime());
    REQUIRE(a.getPid() == b.getPid());
    REQUIRE(a.getScheduledStartTime() == b.getScheduledStartTime());
    REQUIRE(a.getNeuronStateSystem() == b.getNeuronStateSystem());
    REQUIRE(a.getNeededRunTime() == b.getNeededRunTime());
    REQUIRE(a.getNeededCores() == b.getNeededCores());

}

TEST_CASE("Process JSON  works properly", "[simulated_process]") {
    SimProcess<int> p(1,32,15,0.0,-1);
    using nlohmann::json;
    json j = p;
    INFO ("JS: " << j << "\n" );
    auto p2 = j.get<SimProcess<int>>();
    check_p(p,p2);
}

TEST_CASE("Process QUEUE JSON WORKS", "[proc_queue]"){
    int max_time = 128;
    std::default_random_engine genz;
    std::uniform_int_distribution<int> distribution(32, max_time);
    std::uniform_int_distribution<int> core_dist(16, 4096);
    std::uniform_int_distribution<int> pid_dist(1, 8192);
    auto cores_needed = std::bind(core_dist, genz);
    auto time_needed = std::bind(distribution, genz);
    auto pid = std::bind(pid_dist, genz);
    int p_cores_needed = cores_needed();
    int p_time_needed = time_needed();
    int p_pid = pid();
    SimProcessQueue q;
    SECTION("Queue to JSON"){
        SimProcess<int> p(p_pid,p_cores_needed,p_time_needed,0,0);
        q.enqueue(p);
        json j = q.to_json_obj();
        INFO("JS: " << j);
        std::cout << j;
        SimProcessQueue q2;
        q2.from_json_obj(j);

    }
}