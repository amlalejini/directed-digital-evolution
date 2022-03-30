#define CATCH_CONFIG_MAIN

#include "Catch/single_include/catch2/catch.hpp"

#include <unordered_set>

#include "emp/datastructs/vector_utils.hpp"

#include "dirdevo/ExperimentSetups/AvidaGP/AvidaGPTaskSet.hpp"

TEST_CASE("L9 TaskSet", "[l9][TaskSet]")
{

  dirdevo::AvidaGPTaskSet task_set;

  task_set.AddTasksByName({
    "ECHO",
    "NOT",
    "NAND",
    "OR_NOT",
    "AND",
    "OR",
    "AND_NOT",
    "NOR",
    "XOR",
    "EQU"
  });

  CHECK(task_set.GetSize() == 10);
  CHECK(task_set.HasTask("ECHO"));
  CHECK(task_set.HasTask("NOT"));
  CHECK(task_set.HasTask("NAND"));
  CHECK(task_set.HasTask("OR_NOT"));
  CHECK(task_set.HasTask("AND"));
  CHECK(task_set.HasTask("OR"));
  CHECK(task_set.HasTask("AND_NOT"));
  CHECK(task_set.HasTask("NOR"));
  CHECK(task_set.HasTask("XOR"));
  CHECK(task_set.HasTask("EQU"));

  auto& echo_task = task_set.GetTask(task_set.GetID("ECHO"));
  CHECK(echo_task.calc_output_fun({0}) == 0);

}
