#pragma once

#include <string>
#include <algorithm>
#include <functional>
#include <map>
#include <unordered_set>
#include <set>

#include "../../utility/TaskSet.hpp"
#include "../../utility/boolean_logic_impls.hpp"
#include "../../utility/math_task_impls.hpp"

namespace dirdevo {

class AvidaGPTaskSet : public TaskSet<double, double> {
public:
  using this_t = AvidaGPTaskSet;
  using input_t = double;
  using output_t = double;

protected:

  struct AGP_TaskSpec {
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
const std::map<std::string,AvidaGPTaskSet::AGP_TaskSpec> AvidaGPTaskSet::valid_tasks={
  //============================== BOOLEAN LOGIC TASKS ==============================
  {
    "ECHO",
    {
      "ECHO",
       [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { emp_assert(inputs.size() >= 1); return logic::ECHO((uint32_t)inputs[0]); },
       1,
       "ECHO function"
    }
  },

  {
    "NAND",
    {
      "NAND",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { emp_assert(inputs.size() >= 2); return logic::NAND((uint32_t)inputs[0], (uint32_t)inputs[1]); },
      2,
      "NAND boolean logic function"
    }
  },

  {
    "NOT",
    {
      "NOT",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { emp_assert(inputs.size() >= 1); return logic::NOT((uint32_t)inputs[0]); },
      1,
      "NOT boolean logic function"
    }
  },

  {
    "OR_NOT",
    {
      "OR_NOT",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { emp_assert(inputs.size() >= 2); return logic::OR_NOT((uint32_t)inputs[0], (uint32_t)inputs[1]); },
      2,
      "OR_NOT boolean logic function"
    }
  },

  {
    "AND",
    {
      "AND",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { emp_assert(inputs.size() >= 2); return logic::AND((uint32_t)inputs[0], (uint32_t)inputs[1]); },
      2,
      "AND boolean logic function"
    }
  },

  {
    "OR",
    {
      "OR",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { emp_assert(inputs.size() >= 2); return logic::OR((uint32_t)inputs[0], (uint32_t)inputs[1]); },
      2,
      "OR boolean logic function"
    }
  },

  {
    "AND_NOT",
    {
      "AND_NOT",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { emp_assert(inputs.size() >= 2); return logic::AND_NOT((uint32_t)inputs[0], (uint32_t)inputs[1]); },
      2,
      "AND_NOT boolean logic function"
    }
  },

  {
    "NOR",
    {
      "NOR",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { emp_assert(inputs.size() >= 2); return logic::NOR((uint32_t)inputs[0], (uint32_t)inputs[1]); },
      2,
      "NOR boolean logic function"
    }
  },

  {
    "XOR",
    {
      "XOR",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { emp_assert(inputs.size() >= 2); return logic::XOR((uint32_t)inputs[0], (uint32_t)inputs[1]); },
      2,
      "XOR boolean logic function"
    }
  },

  {
    "EQU",
    {
      "EQU",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { emp_assert(inputs.size() >= 2); return logic::EQU((uint32_t)inputs[0], (uint32_t)inputs[1]); },
      2,
      "EQU boolean logic function"
    }
  },

  //============================== 1-INPUT MATH TASKS ==============================
  {
    "MATH_1AA",
    {
      "MATH_1AA",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { return MATH_1IN::AA(inputs[0]); },
      1,
      "1AA"
    }
  },
  {
    "MATH_1AB",
    {
      "MATH_1AB",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { return MATH_1IN::AB(inputs[0]); },
      1,
      "1AB"
    }
  },
  {
    "MATH_1AC",
    {
      "MATH_1AC",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { return MATH_1IN::AC(inputs[0]); },
      1,
      "1AC"
    }
  },
  {
    "MATH_2AA",
    {
      "MATH_2AA",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { return MATH_2IN::AA(inputs[0], inputs[1]); },
      2,
      "2AA"
    }
  },
  {
    "MATH_2AB",
    {
      "MATH_2AB",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { return MATH_2IN::AB(inputs[0], inputs[1]); },
      2,
      "2AB"
    }
  },
  {
    "MATH_2AC",
    {
      "MATH_2AC",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { return MATH_2IN::AC(inputs[0], inputs[1]); },
      2,
      "2AC"
    }
  },
  {
    "MATH_2AD",
    {
      "MATH_2AD",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { return MATH_2IN::AD(inputs[0], inputs[1]); },
      2,
      "2AD"
    }
  },
  {
    "MATH_2AE",
    {
      "MATH_2AE",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { return MATH_2IN::AE(inputs[0], inputs[1]); },
      2,
      "2AE"
    }
  },
  {
    "MATH_2AF",
    {
      "MATH_2AF",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { return MATH_2IN::AF(inputs[0], inputs[1]); },
      2,
      "2AF"
    }
  },
  {
    "MATH_2AG",
    {
      "MATH_2AG",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { return MATH_2IN::AG(inputs[0], inputs[1]); },
      2,
      "2AG"
    }
  },
  {
    "MATH_2AH",
    {
      "MATH_2AH",
      [](const emp::vector<AvidaGPTaskSet::input_t>& inputs) { return MATH_2IN::AH(inputs[0], inputs[1]); },
      2,
      "2AH"
    }
  }

};

} // end dirdevo namespace