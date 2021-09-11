#pragma once

#include <string>
#include <algorithm>
#include <functional>
#include <map>
#include <unordered_set>
#include <set>

#include "../../utility/TaskSet.hpp"
#include "../../utility/boolean_logic_impls.hpp"

namespace dirdevo {

class L9TaskSet : public TaskSet<uint32_t, uint32_t> {
public:
  using this_t = L9TaskSet;
  using input_t = uint32_t;
  using output_t = uint32_t;

protected:

  struct AGP_TaskSpec {
    using input_t = uint32_t;
    using output_t = uint32_t;
    using calc_fun_t = std::function<output_t(const emp::vector<input_t>&)>;
    std::string name;
    calc_fun_t calc;
    size_t num_inputs;
    std::string desc;
    AGP_TaskSpec(const std::string& a_name, const calc_fun_t& a_calc, size_t a_num_inputs, const std::string& a_desc) :
      name(a_name), calc(a_calc), num_inputs(a_num_inputs), desc(a_desc)
    {}
  };

  static const std::map<std::string,AGP_TaskSpec> valid_tasks;

public:

  /// Add tasks from a set of valid pre-defined tasks
  void AddTasksByName(const emp::vector<std::string>& names) {
    std::vector<std::string> unused_names;
    for (const std::string& name : names) {
      if (emp::Has(this_t::valid_tasks, name)) {
        auto& task_spec = this_t::valid_tasks.at(name);
        AddTask(
          task_spec.name,
          task_spec.calc,
          task_spec.num_inputs,
          task_spec.desc
        );
      } else {
        unused_names.emplace_back(name);
      }
    }
    emp_assert(unused_names.size()==0, unused_names);
  }

};

/// Valid tasks for the avidagp task set
const std::map<std::string,L9TaskSet::AGP_TaskSpec> L9TaskSet::valid_tasks={
  //============================== BOOLEAN LOGIC TASKS ==============================
  {
    "ECHO",
    {
      "ECHO",
       [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 1); return logic::ECHO(inputs[0]); },
       1,
       "ECHO function"
    }
  },

  {
    "NAND",
    {
      "NAND",
      [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::NAND(inputs[0], inputs[1]); },
      2,
      "NAND boolean logic function"
    }
  },

  {
    "NOT",
    {
      "NOT",
      [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 1); return logic::NOT(inputs[0]); },
      1,
      "NOT boolean logic function"
    }
  },

  {
    "OR_NOT",
    {
      "OR_NOT",
      [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::OR_NOT(inputs[0], inputs[1]); },
      2,
      "OR_NOT boolean logic function"
    }
  },

  {
    "AND",
    {
      "AND",
      [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::AND(inputs[0], inputs[1]); },
      2,
      "AND boolean logic function"
    }
  },

  {
    "OR",
    {
      "OR",
      [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::OR(inputs[0], inputs[1]); },
      2,
      "OR boolean logic function"
    }
  },

  {
    "AND_NOT",
    {
      "AND_NOT",
      [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::AND_NOT(inputs[0], inputs[1]); },
      2,
      "AND_NOT boolean logic function"
    }
  },

  {
    "NOR",
    {
      "NOR",
      [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::NOR(inputs[0], inputs[1]); },
      2,
      "NOR boolean logic function"
    }
  },

  {
    "XOR",
    {
      "XOR",
      [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::XOR(inputs[0], inputs[1]); },
      2,
      "XOR boolean logic function"
    }
  },

  {
    "EQU",
    {
      "EQU",
      [](const emp::vector<uint32_t>& inputs) { emp_assert(inputs.size() == 2); return logic::EQU(inputs[0], inputs[1]); },
      2,
      "EQU boolean logic function"
    }
  }

  //============================== ARITHMETIC TASKS ==============================

};

} // end dirdevo namespace