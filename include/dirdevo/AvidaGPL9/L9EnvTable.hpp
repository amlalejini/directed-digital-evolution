#pragma once

#include <unordered_set>
#include <unordered_map>
#include <utility>

#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/set_utils.hpp"

#include "L9TaskSet.hpp"

/// Precompute l9 environments to be used during an experiment
namespace dirdevo {

/// Bank of L9 instances
/// TODO - save and load functionality
class L9InstanceBank {
public:

  using this_t = L9InstanceBank;
  using input_t = uint32_t;
  using output_t = uint32_t;
  using task_set_t = L9TaskSet;

  static constexpr input_t MIN_LOGIC_TASK_INPUT=0;
  static constexpr output_t MAX_LOGIC_TASK_INPUT=1000000000;

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
  emp::vector<Environment> environment_bank; ///< Bank of environments.

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
        std::make_pair(output_value, task_id)
      );
    }
  }

  Environment BuildEnvironment() {
    Environment env;
    env.is_collision=true;
    while (env.is_collision) {
      env.Clear();
      env.correct_outputs.resize(task_set.GetSize(), (uint32_t)-1);
      env.input_buffer = {
        random.GetUInt(this_t::MIN_LOGIC_TASK_INPUT, this_t::MAX_LOGIC_TASK_INPUT),
        random.GetUInt(this_t::MIN_LOGIC_TASK_INPUT, this_t::MAX_LOGIC_TASK_INPUT)
      };
      for (size_t task_id = 0; task_id < task_set.GetSize(); ++task_id) {
        const output_t task_output = task_set.GetTask(task_id).calc_output_fun(env.input_buffer);
        SetEnvOutput(env, task_id, task_output);
        // Have we seen this output before? Yes, mark collision and break.
        if (env.is_collision) {
          break;
        }
      }
    }
    return env;
  }

  /// Internal function. Generate count number of task environment instances, adding each to the environment bank.
  /// Each environment is guaranteed to have unique outputs for teach possible task.
  void Generate(size_t count) {
    // TODO - guarantee that every environment is unique?
    // std::unordered_set<std::pair<input_t,input_t>> env_buffers; // Use to make sure each environment is unique
    for (size_t n = 0; n = count; n++) {
      environment_bank.emplace_back(BuildEnvironment());
    }
  }

public:

  L9InstanceBank(
    emp::Random& rnd,
    task_set_t& a_task_set,
    size_t num_environments
  ) :
    random(rnd),
    task_set(a_task_set)
  {
    emp_assert(num_environments > 0, "Can't have a size-0 environment bank.");
    Generate(num_environments);
  }

  size_t GetSize() const { return environment_bank.size(); }

  Environment GetRandEnv() {
    emp_assert(GetSize(), "Environment bank is empty", GetSize());
    return environment_bank[random.GetUInt(environment_bank.size())];
  }

};

}