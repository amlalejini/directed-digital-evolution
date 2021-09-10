#pragma once

#include <string>
#include <algorithm>
#include <functional>
#include <map>
#include <unordered_set>

#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"

namespace dirdevo {

// TODO - clean up language world task vs organism task
/// TaskSet defines a set of Tasks where each task can be identified by a numeric ID and a string name.
/// A Task defines a mapping from a given set (vector) of inputs (INPUT_T) to a given output (OUTPUT_T).
/// This mapping is specified by the task's calc_output_fun, which takes a vector of inputs and maps it to the correct output for the given task.
template<typename INPUT_T, typename OUTPUT_T>
class TaskSet {

public:

  using input_t = INPUT_T;
  using output_t = OUTPUT_T;
  using calc_output_fun_t = std::function<output_t(const emp::vector<input_t>&)>;

  struct Task {
    size_t id;          // Task id
    std::string name;
    calc_output_fun_t calc_output_fun;
    size_t num_inputs;
    std::string desc;

    Task(
      size_t a_id,
      const std::string& a_name,
      const calc_output_fun_t& a_calc_output_fun,
      size_t a_num_inputs,
      const std::string& a_desc
    ) :
      id(a_id),
      name(a_name),
      calc_output_fun(a_calc_output_fun),
      num_inputs(a_num_inputs),
      desc(a_desc)
     { ; }

  };

  /// Defines a single instance of a task. I.e., a particular set of inputs and the correct output as calculated by the Task definition.
  struct Instance {
    size_t task_id;
    emp::vector<input_t> inputs;
    output_t output;              // Correct outputs for the given inputs.
  };

protected:

  emp::vector<Task> task_lib;
  std::map<std::string, size_t> name_map;

public:

  size_t GetSize() const { return task_lib.size(); }
  const std::string& GetName(size_t id) const { return task_lib[id].name; }
  const std::string& GetDesc(size_t id) const { return task_lib[id].desc; }

  size_t GetID(const std::string& name) const {
    emp_assert(emp::Has(name_map, name), name);
    return emp::Find(name_map, name, (size_t)-1);
  }

  /// Retrieve a task by ID
  const Task& GetTask(size_t id) const { return task_lib[id]; }

  bool HasTask(const std::string& name) const { return emp::Has(name_map, name); }

  /// Add a new task to the task set.
  void AddTask(
    const std::string& name,
    const calc_output_fun_t& calc_output_fun,
    size_t num_inputs,
    const std::string& desc =""
  ) {
    const size_t id = task_lib.size();
    task_lib.emplace_back(
      id,
      name,
      calc_output_fun,
      num_inputs,
      desc
    );
    name_map[name] = id;
  }

  /// Reset the task set.
  void Clear() {
    task_lib.clear();
    name_map.clear();
  }

};

} // end dirdevo namespace

// #endif