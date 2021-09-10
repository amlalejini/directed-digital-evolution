#pragma once

#include <string>
#include <algorithm>
#include <functional>
#include <map>
#include <unordered_set>

#include "../../utility/TaskSet.hpp"
#include "../../utility/boolean_logic_impls.hpp"

namespace dirdevo {


/// This function takes a task set (should derive from TaskSet), and adds 9 one and two input boolean logic operations as tasks (plus ECHO).
template<typename TASK_SET>
void AddLogic9Tasks(TASK_SET& task_set) {
  // ECHO
  task_set.AddTask(
    "ECHO",
    [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 1); return logic::ECHO(inputs[0]); },
    1,
    "ECHO boolean logic function"
  );
  // NAND
  task_set.AddTask(
    "NAND",
    [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::NAND(inputs[0], inputs[1]); },
    2,
    "NAND boolean logic function"
  );
  // NOT
  task_set.AddTask(
    "NOT",
    [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 1); return logic::NOT(inputs[0]); },
    1,
    "NOT boolean logic function"
  );
  // OR_NOT
  task_set.AddTask(
    "OR_NOT",
    [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::OR_NOT(inputs[0], inputs[1]); },
    2,
    "OR_NOT boolean logic function"
  );
  // AND
  task_set.AddTask(
    "AND",
    [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::AND(inputs[0], inputs[1]); },
    2,
    "AND boolean logic function"
  );
  // OR
  task_set.AddTask(
    "OR",
    [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::OR(inputs[0], inputs[1]); },
    2,
    "OR boolean logic function"
  );
  // AND_NOT
  task_set.AddTask(
    "AND_NOT",
    [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::AND_NOT(inputs[0], inputs[1]); },
    2,
    "AND_NOT boolean logic function"
  );
  // NOR
  task_set.AddTask(
    "NOR",
    [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::NOR(inputs[0], inputs[1]); },
    2,
    "NOR boolean logic function"
  );
  // XOR
  task_set.AddTask(
    "XOR",
    [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::XOR(inputs[0], inputs[1]); },
    2,
    "XOR boolean logic function"
  );
  // EQU
  task_set.AddTask(
    "EQU",
    [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::EQU(inputs[0], inputs[1]); },
    2,
    "EQU boolean logic function"
  );
}

class L9TaskSet : public TaskSet<uint32_t, uint32_t> {
public:
  using this_t = L9TaskSet;
  using input_t = uint32_t;
  using output_t = uint32_t;

protected:
public:
  L9TaskSet() {
    // Add logic 9 tasks to this task set
    dirdevo::AddLogic9Tasks<L9TaskSet>(*this);
  }
};

} // end dirdevo namespace