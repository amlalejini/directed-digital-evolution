#define CATCH_CONFIG_MAIN

#include "Catch/single_include/catch2/catch.hpp"

#include <unordered_set>

#include "emp/math/Random.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/set_utils.hpp"

#include "dirdevo/ExperimentSetups/AvidaGPL9/L9TaskSet.hpp"
#include "dirdevo/ExperimentSetups/AvidaGPL9/L9EnvironmentBank.hpp"

TEST_CASE("L9EnvironmentBank", "[l9]")
{
  constexpr size_t seed=2;
  dirdevo::L9TaskSet task_set;
  emp::Random random(seed);

  // Create a size-10 environment bank
  dirdevo::L9EnvironmentBank env_bank10(random, task_set, 10);
  CHECK(env_bank10.GetSize() == 10);
  // Is each environment collision-free?
  for (size_t i = 0; i < env_bank10.GetSize(); ++i) {
    auto env = env_bank10.GetEnvironment(i);
    CHECK(!env.is_collision);
    CHECK(env.valid_outputs.size() == task_set.GetSize());
    for (size_t task_id = 0; task_id < task_set.GetSize(); ++task_id) {
      auto& task = task_set.GetTask(task_id);

      const uint32_t calc_task_output = task.calc_output_fun(
        (task.num_inputs > 1) ? env.input_buffer : emp::vector<uint32_t>({env.input_buffer[0]})
      );
      const uint32_t env_task_output = env.correct_outputs[task_id];
      CHECK(calc_task_output == env_task_output);
      CHECK(emp::Has(env.valid_outputs, calc_task_output));
      CHECK(env.task_lookup[calc_task_output].size() == 1);
    }
  }

  // Create a big environment bank.
  dirdevo::L9EnvironmentBank env_bank10000(random, task_set, 10000);
  CHECK(env_bank10000.GetSize() == 10000);

}
