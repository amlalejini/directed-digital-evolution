

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <sys/stat.h>

#include "emp/config/config.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/config/command_line.hpp"
#include "emp/io/File.hpp"
#include "emp/bits/BitSet.hpp"
#include "emp/bits/BitArray.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/data/DataFile.hpp"

EMP_BUILD_CONFIG(MaxCovConfig,
  VALUE(SEED, int, -1, "Random number generator seed."),
  VALUE(POP_PROFILE_FILE, std::string, "population_profiles.csv", "Path to the environment file that specifies which tasks are rewarded at organism and world level"),
  VALUE(OUTPUT_DIR, std::string, "output", "Where should the experiment dump output?")
)

using csv_line_t = std::unordered_map<std::string, std::string>;
using csv_data_t = emp::vector< csv_line_t >;

constexpr size_t NUM_POP_TASKS=18;

csv_data_t read_csv(std::ifstream& pop_profiles_fstream) {
  emp_assert(pop_profiles_fstream.is_open());
  // Load as empirical File
  emp::File pop_profiles_file(pop_profiles_fstream);
  // Process lines
  // (1) Extract header
  emp::vector<std::string> header = pop_profiles_file.ExtractRowAs<std::string>(',');
  // (2) Extract lines as maps
  auto data = pop_profiles_file.ToData<std::string>(',');
  csv_data_t lines;
  for (size_t data_i = 0; data_i < data.size(); ++data_i) {
    lines.emplace_back();
    for (size_t c = 0; c < data[data_i].size(); ++c) {
      lines.back()[header[c]] = data[data_i][c];
    }
  }
  pop_profiles_fstream.close();
  return lines;
}

struct PopulationInfo {
  size_t pop_idx;           // Index of this pop info in the metapopulation info
  size_t experiment_pop_uid; // Pop ID in experiment
  emp::BitArray<NUM_POP_TASKS> covered_tasks;
  size_t num_tasks_covered=0;

  PopulationInfo(
    size_t in_pop_idx=0,
    size_t in_exp_pop_uid=0
  ) :
    pop_idx(in_pop_idx),
    experiment_pop_uid(in_exp_pop_uid)
  {
    covered_tasks.Clear();
  }

};

struct TaskInfo {
  size_t task_id;
  std::string task_name;
  TaskInfo(
    size_t in_task_id=0,
    const std::string& in_task_name=""
  ) :
    task_id(in_task_id),
    task_name(in_task_name)
  { ; }
};

struct SolutionInfo {
  size_t num_pops=0;
  size_t max_tasks_covered=0;

  emp::vector<size_t> pop_idxs;
  emp::BitArray<NUM_POP_TASKS> covered_tasks;

  SolutionInfo() { covered_tasks.Clear(); }
  SolutionInfo(const SolutionInfo& other) :
    num_pops(other.num_pops),
    max_tasks_covered(other.max_tasks_covered),
    pop_idxs(other.pop_idxs),
    covered_tasks(other.covered_tasks)
  {}
};

struct MetapopulationInfo {
  int seed;
  int epoch;
  std::string selection_method;

  emp::vector<TaskInfo> tasks;
  // emp::vector< std::unordered_set<size_t> >

  std::unordered_map<std::string, size_t> task_name_to_id;

  size_t num_tasks_covered=0;
  emp::BitArray<NUM_POP_TASKS> covered_tasks; // TODO - try left/right orientation

  std::unordered_map<size_t, size_t> pop_uids_to_idx;
  emp::vector<PopulationInfo> pops;

  MetapopulationInfo(
    int in_seed=0,
    int in_epoch=0,
    const std::string& in_selection_method=""
  ) :
    seed(in_seed),
    epoch(in_epoch),
    selection_method(in_selection_method)
  {
    covered_tasks.Clear();
    // tasks.resize(NUM_POP_TASKS);
  }

  void AddLine(csv_line_t& line) {
    const size_t exp_pop_uid = emp::from_string<size_t>(line["pop_id"]);
    size_t pop_idx = 0;

    // Add (genera) task information
    bool task_covered = line["task_coverage"] == "1";
    const std::string& task_name = line["task_name"];
    size_t task_idx = 0;
    // If first time seeing this task for metapopulation, track it.
    if (!emp::Has(task_name_to_id, task_name)) {
      // task_idx = emp::from_string<size_t>(line["task_id"]);
      task_idx = tasks.size();
      tasks.emplace_back(
        task_idx,
        task_name
      );
      task_name_to_id[task_name] = task_idx;
      emp_assert(task_idx < NUM_POP_TASKS);
    } else {
      task_idx = task_name_to_id[task_name];
    }
    num_tasks_covered += (size_t)(task_covered & !covered_tasks[task_idx]);


    // If first time seeing population, add to pops.
    if (!emp::Has(pop_uids_to_idx, exp_pop_uid)) {
      pop_idx = pops.size();
      pop_uids_to_idx.emplace(std::make_pair(exp_pop_uid, pop_idx));
      pops.emplace_back(
        pop_idx,
        exp_pop_uid
      );
    } else {
      pop_idx = pop_uids_to_idx[exp_pop_uid];
    }

    if (task_covered) {
      covered_tasks.Set(task_idx);
      pops[pop_idx].num_tasks_covered += (size_t)(!(pops[pop_idx].covered_tasks[task_idx]));
      pops[pop_idx].covered_tasks.Set(task_idx);
    }

  }
};

// Notes
// - Can solve for each metapopulation independently
// - Use smaller solutions to inform bound for

struct SolveState {
  std::unordered_set<size_t> undecided;
  emp::vector<size_t> include;
  emp::vector<size_t> exclude;
  size_t num_tasks_covered=0;
  emp::BitArray<NUM_POP_TASKS> covered_tasks;


  SolveState() { covered_tasks.Clear(); }
  SolveState(const SolveState& other) :
    undecided(other.undecided),
    include(other.include),
    exclude(other.exclude),
    num_tasks_covered(other.num_tasks_covered),
    covered_tasks(other.covered_tasks)
  { }

};

// todo - add return type
emp::vector<SolutionInfo> solver(MetapopulationInfo& metapop) {
  emp::vector<SolutionInfo> solutions(metapop.pops.size());
  const size_t max_possible_coverage = metapop.num_tasks_covered;
  auto& tasks = metapop.tasks;
  auto& pops = metapop.pops;
  //////////////////////////////////////////////////////////////
  // Given a number of populations, find maximum task coverage.
  //////////////////////////////////////////////////////////////
  // - Compress redundant populations (choose 1 representative of each profile)
  std::unordered_set<std::string> unique_profiles;
  emp::vector<size_t> unique_pop_ids;
  for (size_t pop_i = 0; pop_i < pops.size(); ++pop_i) {
    std::string profile(pops[pop_i].covered_tasks.ToString());
    if (emp::Has(unique_profiles, profile)) continue;
    unique_pop_ids.emplace_back(pop_i);
    unique_profiles.emplace(profile);
  }
  emp_assert(unique_profiles.size() > 0);
  emp_assert(unique_pop_ids.size() > 0);

  // -- Prep computations --
  // - Find population with the maximum task coverage.
  size_t best_single_cov_pop_id=unique_pop_ids[0];
  size_t best_single_pop_cov=pops[best_single_cov_pop_id].num_tasks_covered;
  for (size_t pop_i : unique_pop_ids) {
    const size_t cur_pop_cov = pops[pop_i].num_tasks_covered;
    if (cur_pop_cov > best_single_pop_cov) {
      best_single_cov_pop_id = pop_i;
      best_single_pop_cov = cur_pop_cov;
    }
  }

  // - Track which populations cover each task.
  emp::vector< std::unordered_set<size_t> > pop_set_that_cover_each_task(tasks.size());
  emp::vector< emp::vector<size_t> > pops_that_cover_each_task(tasks.size());
  for (size_t task_i = 0; task_i < tasks.size(); ++task_i) {
    for (size_t pop_i : unique_pop_ids) {
      if (pops[pop_i].covered_tasks[task_i]) {
        pops_that_cover_each_task[task_i].emplace_back(pop_i);
        pop_set_that_cover_each_task[task_i].emplace(pop_i);
      }
    }
  }

  // - Solve n=1 (best single pop cov)
  solutions[0].num_pops = 1;
  solutions[0].max_tasks_covered = best_single_pop_cov;
  solutions[0].pop_idxs.emplace_back(best_single_cov_pop_id);
  solutions[0].covered_tasks = pops[best_single_cov_pop_id].covered_tasks;

  // -- Solve --
  size_t prev_solution_i = 0;
  for (size_t sol_i = 1; sol_i < solutions.size(); ++sol_i) {
    auto& prev_sol = solutions[prev_solution_i];
    auto& sol = solutions[sol_i];
    sol.num_pops = prev_sol.num_pops+1;
    const size_t N = sol.num_pops;
    prev_solution_i = sol_i; // Update this here so we can shortcut loop w/out worrying about updating this later.

    // Can we solve for this N trivially using previous solution (for N-1)?
    // If prev solution coverage == metapop task coverage, we can do no better than previous solution.
    if (prev_sol.max_tasks_covered == max_possible_coverage) {
      // This solution == previous solution
      sol.max_tasks_covered = prev_sol.max_tasks_covered;
      emp_assert(sol.max_tasks_covered == max_possible_coverage);
      sol.pop_idxs = prev_sol.pop_idxs;
      sol.covered_tasks = prev_sol.covered_tasks;
      emp_assert(sol.covered_tasks.EQU(metapop.covered_tasks).All(), sol.covered_tasks.CountOnes(), metapop.covered_tasks.CountOnes());
      continue;
    } else if (prev_sol.max_tasks_covered + 1 == max_possible_coverage) {
      // Previous solution is only one task away from the maximum possible coverage.
      // If it was possible to find max possible coverage with fewer populations, we would have done so already.
      // AND if there's only one task not covered (out of those that are covered by the metapopulation), there MUST be at least population that performs that task.
      emp_assert((metapop.covered_tasks.XOR(prev_sol.covered_tasks)).CountOnes() == 1);
      int missing_task_id = (metapop.covered_tasks.XOR(prev_sol.covered_tasks)).FindOne();
      emp_assert((size_t)missing_task_id < tasks.size());
      sol.max_tasks_covered = max_possible_coverage;
      sol.pop_idxs = prev_sol.pop_idxs;
      emp_assert(pops_that_cover_each_task[(size_t)missing_task_id].size());
      sol.pop_idxs.emplace_back( pops_that_cover_each_task[(size_t)missing_task_id][0] );
      sol.covered_tasks = metapop.covered_tasks;
      continue;
    }

    // Create a reasonable bound by taking the previous solution and adding the population that adds the most to task coverage.
    SolutionInfo bound(prev_sol);
    // bound.num_pops = prev_sol.num_pops;
    // bound.pop_idxs = prev_sol.pop_idxs
    emp_assert(prev_sol.covered_tasks.EQU(bound.covered_tasks).All());
    std::unordered_set<size_t> bound_pops(
      bound.pop_idxs.begin(),
      bound.pop_idxs.end()
    );
    int add_id = -1;
    size_t add_value = 0;
    for (size_t pop_i : unique_pop_ids) {
      if (emp::Has(bound_pops, pop_i)) continue;
      const size_t cur_add_value = bound.covered_tasks.OR(pops[pop_i].covered_tasks).CountOnes();
      if (cur_add_value > add_value) {
        add_value = cur_add_value;
        add_id = pop_i;
      }
    }
    emp_assert(add_id > -1);
    bound.num_pops += 1;
    bound.covered_tasks = bound.covered_tasks.OR(pops[add_id].covered_tasks);
    bound.pop_idxs.emplace_back((size_t)add_id);
    bound.max_tasks_covered = bound.covered_tasks.CountOnes();
    emp_assert(bound.pop_idxs.size() == N);
    // If by adding a single population, we hit the maximum possible coverage, we've solved for this num of pops (can't do better).
    if (bound.max_tasks_covered == max_possible_coverage) {
      sol.covered_tasks = bound.covered_tasks;
      sol.pop_idxs = bound.pop_idxs;
      sol.max_tasks_covered = bound.max_tasks_covered;
      continue;
    }


    // -- Start brute force --
    // Create initial problem state
    emp::vector<SolveState> state_stack(1);
    auto& initial_state = state_stack.back();
    for (size_t pop_i : unique_pop_ids) {
      initial_state.undecided.emplace(pop_i);
    }
    emp_assert(initial_state.undecided.size() == unique_pop_ids.size());
    initial_state.include.resize(0);
    initial_state.exclude.resize(0);
    initial_state.covered_tasks.Clear();
    initial_state.num_tasks_covered=0;

    // If fewer than N populations are needed to achieve maximum possible coverage, we should never get to this point.
    emp_assert(unique_pop_ids.size() <= N);
    while (state_stack.size()) {
      // std::cout << "    - State stack size: " << state_stack.size() << std::endl;
      SolveState eval_state(state_stack.back());
      state_stack.pop_back();
      // (1) Is this a leaf? I.e., we've included N populations already.
      if (eval_state.include.size() == N) {
        // Is this better than our current best?
        if (eval_state.num_tasks_covered > bound.max_tasks_covered) {
          bound.max_tasks_covered = eval_state.num_tasks_covered;
          bound.pop_idxs = eval_state.include;
          bound.covered_tasks = eval_state.covered_tasks;
          emp_assert(eval_state.covered_tasks.CountOnes() == eval_state.num_tasks_covered);
        }
        continue;
      // (2) If we don't have enough undecided to get up to the target N, prune this branch.
      } else if (eval_state.include.size() + eval_state.undecided.size() < N) {
        continue;
      // (3) If we have just enough undecided to get to N, what is the best we can do?
      } else if (eval_state.include.size() + eval_state.undecided.size() == N) {
        // I.e., add all undecided to included
        for (size_t pop_i : eval_state.undecided) {
          eval_state.include.emplace_back(pop_i);
          eval_state.covered_tasks.OR_SELF(pops[pop_i].covered_tasks);
        }
        eval_state.undecided.clear();
        eval_state.num_tasks_covered = eval_state.covered_tasks.CountOnes();
        if (eval_state.num_tasks_covered > bound.max_tasks_covered) {
          bound.max_tasks_covered = eval_state.num_tasks_covered;
          bound.pop_idxs = eval_state.include;
          bound.covered_tasks = eval_state.covered_tasks;
          emp_assert(bound.pop_idxs.size() == N);
          emp_assert(eval_state.covered_tasks.CountOnes() == eval_state.num_tasks_covered);
        }
        continue;
      // (4) If we can only add one more population, go ahead and solve this branch.
      } else if (eval_state.include.size() + 1 == N) {
        int best_cov = -1;
        size_t best_cov_i = 0;
        for (size_t pop_i : eval_state.undecided) {
          int cov = (int)(eval_state.covered_tasks.OR(pops[pop_i].covered_tasks)).CountOnes();
          if (cov > best_cov) {
            best_cov_i = pop_i;
            best_cov = cov;
          }
        }
        eval_state.include.emplace_back(best_cov_i);
        eval_state.covered_tasks.OR_SELF(pops[best_cov_i].covered_tasks);
        eval_state.num_tasks_covered = eval_state.covered_tasks.CountOnes();
        if (eval_state.num_tasks_covered > bound.max_tasks_covered) {
          bound.max_tasks_covered = eval_state.num_tasks_covered;
          bound.pop_idxs = eval_state.include;
          bound.covered_tasks = eval_state.covered_tasks;
          emp_assert(bound.pop_idxs.size() == N);
          emp_assert(eval_state.covered_tasks.CountOnes() == eval_state.num_tasks_covered);
        }
        continue;
      }


      emp_assert(eval_state.undecided.size() > 0);

      // ========================
      // If we were to include *everything*, would we do better than the current best?
      emp::BitArray<NUM_POP_TASKS> max_possible(eval_state.covered_tasks);
      for (size_t pop_i : eval_state.undecided) {
        max_possible.OR_SELF(pops[pop_i].covered_tasks);
      }
      // If not, prune this branch.
      if (max_possible.CountOnes() <= bound.max_tasks_covered) {
        continue;
      }
      // ========================

      // ========================
      // Automatically exclude anything that will not improve our score.
      emp::vector<size_t> auto_exclude;
      for (size_t pop_i : eval_state.undecided) {
        size_t pot_score = eval_state.covered_tasks.OR(pops[pop_i].covered_tasks).CountOnes();
        emp_assert(pot_score >= eval_state.num_tasks_covered);
        if (pot_score <= eval_state.num_tasks_covered) {
          auto_exclude.emplace_back(pop_i);
        }
      }
      for (size_t pop_i : auto_exclude) {
        eval_state.exclude.emplace_back(pop_i);
        eval_state.undecided.erase(pop_i);
      }
      // If automatic excludes make this an invalid configuration, prune.
      if (eval_state.include.size() + eval_state.undecided.size() < N) {
        continue;
      }
      // ========================

      // ========================
      // Identify next population to include/exclude
      // - Pick the population that will add the most coverage
      emp::vector<size_t> undecided(
        eval_state.undecided.begin(),
        eval_state.undecided.end()
      );
      // std::cout << undecided << std::endl;
      size_t next_pop = undecided[0];
      size_t next_cov = eval_state.covered_tasks.OR(pops[next_pop].covered_tasks).CountOnes();
      for (size_t i = 1; i < undecided.size(); ++i) {
        size_t p = undecided[i];
        const size_t cov = eval_state.covered_tasks.OR(pops[p].covered_tasks).CountOnes();
        if (cov > next_cov) {
          next_cov = cov;
          next_pop = p;
        }
      }
      // ========================

      // Try excluding 'next pop'
      SolveState exclude_state(eval_state);
      exclude_state.exclude.emplace_back(next_pop);
      exclude_state.undecided.erase(next_pop);
      state_stack.emplace_back(exclude_state);

      // Try including 'next pop'
      eval_state.include.emplace_back(next_pop);
      eval_state.covered_tasks.OR_SELF(pops[next_pop].covered_tasks);
      eval_state.num_tasks_covered = eval_state.covered_tasks.CountOnes();
      eval_state.undecided.erase(next_pop);
      state_stack.emplace_back(eval_state);
    }

    // Bound contains the best thing we ever found
    sol.covered_tasks = bound.covered_tasks;
    sol.pop_idxs = bound.pop_idxs;
    sol.max_tasks_covered = bound.max_tasks_covered;

    emp_assert(sol.num_pops==N);
  }


  //////////////////////////////////////////////////////////////
  // Clean up solutions (add convenient info)
  //////////////////////////////////////////////////////////////
  // TODO

  // add an n=0 solution to make graphing nicer
  solutions.emplace_back();
  return solutions;
}

int main(int argc, char* argv[]) {

  // ==================================================================
  // Read configuration
  std::string config_fname = "max-coverage-config.cfg";
  MaxCovConfig config;
  auto args = emp::cl::ArgManager(argc, argv);
  config.Read(config_fname);
  if (args.ProcessConfigOptions(config, std::cout, config_fname, "max-coverage-config-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0); // If there are leftover args, throw an error.

  // Write to screen how the experiment is configured
  std::cout << "==============================" << std::endl;
  std::cout << "|    How am I configured?    |" << std::endl;
  std::cout << "==============================" << std::endl;
  config.Write(std::cout);
  std::cout << "==============================\n" << std::endl;
  // ==================================================================

  // create output directory if it doesn't exist yet
  std::string output_dir = config.OUTPUT_DIR();
  mkdir(output_dir.c_str(), ACCESSPERMS);
  if(output_dir.back() != '/') {
    output_dir += '/';
  }

  // Read csv file
  std::ifstream pop_profiles_fstream;
  pop_profiles_fstream.open(config.POP_PROFILE_FILE());
  if (!pop_profiles_fstream.is_open()) {
    std::cout << "Failed fo open population profiles csv file: " << config.POP_PROFILE_FILE() << std::endl;
    return -1;
  }
  csv_data_t lines(read_csv(pop_profiles_fstream));
  std::cout << "Num lines: " << lines.size() << std::endl;

  // Organize lines into metapopulations
  std::unordered_map<size_t, MetapopulationInfo> metapopulations;
  for (auto& line : lines) {
    if (line["pop_level"] == "0") {
      continue; // Skip non-population-level tasks.
    }

    // Build metapop identifier
    size_t metapop_identifier = emp::from_string<size_t>(line["SEED"]);

    // If this is the first time we've seen this metapopulation, add it to the set.
    if (!emp::Has(metapopulations, metapop_identifier)) {
      metapopulations[metapop_identifier] = MetapopulationInfo(
        emp::from_string<int>(line["SEED"]),
        emp::from_string<int>(line["epoch"]),
        line["SELECTION_METHOD"]
      );
    }

    // If this is the first time we've seen this population, add it to the set.
    auto& metapop = metapopulations[metapop_identifier];
    metapop.AddLine(line);

  }
  std::cout << "Metapopulations found: " << metapopulations.size() << std::endl;

  // std::unordered_set<std::string> fields


  emp::DataFile output_file(output_dir+"max_coverage_per_pop_cnt.csv");
  emp::Ptr<MetapopulationInfo> out_metapop_ptr=nullptr;
  emp::Ptr<SolutionInfo> out_solution_ptr=nullptr;
  // SEED
  output_file.AddFun<size_t>(
    [&out_metapop_ptr, &out_solution_ptr]() {
      return out_metapop_ptr->seed;
    },
    "SEED"
  );
  // SELECTION
  output_file.AddFun<std::string>(
    [&out_metapop_ptr, &out_solution_ptr]() {
      return out_metapop_ptr->selection_method;
    },
    "SELECTION_METHOD"
  );
  // metapop_size (if there)
  output_file.AddFun<size_t>(
    [&out_metapop_ptr, &out_solution_ptr]() {
      return out_metapop_ptr->pops.size();
    },
    "metapop_size"
  );
  // n_pops
  output_file.AddFun<size_t>(
    [&out_metapop_ptr, &out_solution_ptr]() {
      return out_solution_ptr->num_pops;
    },
    "n_pops"
  );
  // % pops
  output_file.AddFun<double>(
    [&out_metapop_ptr, &out_solution_ptr]() {
      return ((double)out_solution_ptr->num_pops) / (double)out_metapop_ptr->pops.size();
    },
    "n_pops_prop"
  );
  // num tasks
  output_file.AddFun<size_t>(
    [&out_metapop_ptr, &out_solution_ptr]() {
      return out_solution_ptr->max_tasks_covered;
    },
    "max_tasks_covered"
  );
  // % of meta cov
  output_file.AddFun<double>(
    [&out_metapop_ptr, &out_solution_ptr]() {
      return ((double)(out_solution_ptr->max_tasks_covered)) / ((double)(out_metapop_ptr->num_tasks_covered));
    },
    "max_tasks_covered_prop"
  );
  output_file.PrintHeaderKeys();

  // Solve each population for each tas
  // TODO - parallelize!
  size_t prog_counter = 0;
  for (auto& metapop : metapopulations) {
    out_metapop_ptr = &(metapop.second);
    prog_counter += 1;
    std::cout << "Processing metapopulation ("<<prog_counter<<"/"<< metapopulations.size() <<")" << std::endl;
    //////////////////////////////////////////////////////////////////////////////////////////
    // Print metapopulation info
    std::cout << "  MetaPop id: " << metapop.first << "; ";
    std::cout << "Coverage: ";
    metapop.second.covered_tasks.Print();
    std::cout << "; ";
    std::cout << "# cov: " << metapop.second.num_tasks_covered << "; ";
    std::cout << "# pops: " << metapop.second.pops.size() << "; ";
    std::cout << std::endl;
    // Print solutions for each N (num pops)
    emp::vector<SolutionInfo> solutions(solver(metapop.second));
    for (size_t sol_i = 0; sol_i < solutions.size(); ++sol_i) {
      out_solution_ptr = &solutions[sol_i];
      auto& sol = solutions[sol_i];
      std::cout << "  N=" << sol.num_pops << "; ";
      std::cout << "Max cov: " << sol.max_tasks_covered << "; ";
      std::cout << "Coverage: ";
      sol.covered_tasks.Print();
      std::cout << "; ";
      std::cout << std::endl;
      output_file.Update();
    }
    //////////////////////////////////////////////////////////////////////////////////////////

  }


  return 0;
}