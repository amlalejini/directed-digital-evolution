#pragma once

#include <unordered_set>
#include <unordered_map>
#include <utility>

#include "emp/base/assert_warning.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/set_utils.hpp"
#include "emp/base/Ptr.hpp"

#include "AvidaGPTaskSet.hpp"

/// Precompute l9 environments to be used during an experiment
namespace dirdevo {

/// Bank of L9 instances
/// TODO - save and load functionality
class AvidaGPEnvironmentBank {
public:

  using this_t = AvidaGPEnvironmentBank;
  using task_set_t = AvidaGPTaskSet;
  using input_t = typename task_set_t::input_t;
  using output_t = typename task_set_t::output_t;

  static constexpr input_t MIN_LOGIC_TASK_INPUT=0;
  static constexpr input_t MAX_LOGIC_TASK_INPUT=100000000; // max uint32: 4294967295
  static constexpr size_t MAX_ENV_BUILD_TRIES=10000;

  /// Numeric environment (specifies input buffer for an organism + all correct outputs)
  struct Environment {
    emp::vector<input_t> input_buffer;                ///< Species the inputs for this environment instance.
    std::unordered_set<output_t> valid_outputs;       ///< Set of valid output values for this environment.
    emp::vector<output_t> correct_outputs;            ///< Correct outputs indexed by task id.
    std::unordered_map<output_t, emp::vector<size_t>> task_lookup; ///< Which output belongs to which task?
    bool is_collision=false;

    /// clear the environment
    void Clear() {
      input_buffer.clear();
      valid_outputs.clear();
      correct_outputs.clear();
      task_lookup.clear();
      is_collision=false;
    }

    bool operator==(const Environment& other) const {
      return std::tie(
        input_buffer, valid_outputs, correct_outputs, task_lookup
      ) == std::tie(
        other.input_buffer, other.valid_outputs, other.correct_outputs, other.task_lookup
      );
    }

  };

protected:

  emp::Random& random;
  task_set_t& task_set;
  emp::vector<emp::Ptr<Environment>> environment_bank; ///< Bank of environments.

  /// Internal helper function to add a task (task_id) output (output_value) to an environment (env)
  void SetEnvOutput(Environment& env, size_t task_id, output_t output_value) {
    if (task_id >= env.correct_outputs.size()) { env.correct_outputs.resize(task_id+1, 0); }
    env.correct_outputs[task_id] = output_value; // Set correct output for this task id.
    env.is_collision = emp::Has(env.valid_outputs, output_value); // Mark if environment contains an output collision.
    env.valid_outputs.emplace(output_value); // Add output value to valid outputs set.
    // Add output value to task lookup (regardless of whether or not we've seen this output value before)
    if (emp::Has(env.task_lookup, output_value)) {
      env.task_lookup.find(output_value)->second.emplace_back(task_id);
    } else {
      env.task_lookup.emplace(
        std::make_pair(output_value, emp::vector<size_t>({task_id}))
      );
    }
  }

  Environment BuildEnvironment(bool unique_outputs) {
    Environment env;
    env.is_collision=true;
    size_t build_tries = 0;
    do {
      env.Clear();
      env.correct_outputs.resize(task_set.GetSize(), (uint32_t)-1);
      env.input_buffer = {
        (input_t)random.GetUInt(this_t::MIN_LOGIC_TASK_INPUT, this_t::MAX_LOGIC_TASK_INPUT),
        (input_t)random.GetUInt(this_t::MIN_LOGIC_TASK_INPUT, this_t::MAX_LOGIC_TASK_INPUT)
      };
      for (size_t task_id = 0; (task_id < task_set.GetSize()) && !env.is_collision; ++task_id) {
        const auto& task = task_set.GetTask(task_id);
        const output_t task_output = task.calc_output_fun(
          (task.num_inputs > 1) ? env.input_buffer : emp::vector<input_t>({env.input_buffer[0]})
        );
        SetEnvOutput(env, task_id, task_output);
      }
      ++build_tries;
    } while (env.is_collision && unique_outputs && (build_tries < this_t::MAX_ENV_BUILD_TRIES));
    emp_assert_warning(build_tries <= this_t::MAX_ENV_BUILD_TRIES, "Failed to build environment with unique outputs for each task.");
    return env;
  }

public:

  AvidaGPEnvironmentBank(
    emp::Random& rnd,
    task_set_t& a_task_set
  ) :
    random(rnd),
    task_set(a_task_set)
  { ; }

  ~AvidaGPEnvironmentBank() {
    Clear();
  }

  /// Generate count number of task environment instances, adding each to the environment bank.
  /// Each environment is guaranteed to have unique outputs for teach possible task.
  /// WARNING - calling this function will delete any existing environments in this bank, invalidating references to them.
  void GenerateBank(size_t count, bool unique_outputs=true) {
    Clear();
    environment_bank.resize(count, nullptr);
    for (size_t n = 0; n < count; n++) {
      environment_bank[n] = emp::NewPtr<Environment>(BuildEnvironment(unique_outputs));
    }
  }

  void Clear() {
    for (emp::Ptr<Environment> env : environment_bank) {
      env.Delete();
    }
    environment_bank.clear();
  }

  size_t GetSize() const { return environment_bank.size(); }

  const Environment& GetEnvironment(size_t i) const { emp_assert(i < GetSize()); return *(environment_bank[i]); }

  const Environment& GetRandEnv() {
    emp_assert(GetSize(), "Environment bank is empty", GetSize());
    return *(environment_bank[random.GetUInt(environment_bank.size())]);
  }

};

}