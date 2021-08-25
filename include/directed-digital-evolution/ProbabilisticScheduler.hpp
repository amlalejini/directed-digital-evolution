/**
 *
 */

#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_PROBABILISTIC_SCHEDULER_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_PROBABILISTIC_SCHEDULER_HPP_INCLUDE

#include <algorithm>
#include <numeric>

#include "emp/datastructs/IndexMap.hpp"
#include "emp/math/Random.hpp"

namespace dirdevo {

/**
 * ProbabilisticScheduler is based roughly on Austin Ferguson's implementation, here: https://github.com/FergusonAJ/MABE2/blob/avida_basics/source/select/SchedulerProbabilistic.hpp
 *
 * Two ways to use:
 *  - (1) The ProbabilisticScheduler will maintain a schedule (of a configured size), and you can use UpdateSchedule calls to update this maintained schedule based on current item weights.
 *    This approach allows you to repeatedly compute chunks of a schedule without constantly creating new vectors.
 *  - (2) Alternatively, just call GetRandom repeatedly each time you need choose something to 'run'.
 */
class ProbabilisticScheduler {
public:
  using schedule_t = emp::vector<size_t>;

protected:

  emp::Random & random;
  size_t num_items;

  schedule_t schedule;
  emp::IndexMap weight_map;

public:
  ProbabilisticScheduler(
    emp::Random & rnd,
    size_t n_items=0,
    size_t schedule_size=0
  ) :
    random(rnd),
    num_items(n_items),
    schedule(schedule_size),
    weight_map(num_items)
  {
    std::cout << weight_map.GetSize() << std::endl;
    size_t i=0;
    std::generate(
      schedule.begin(),
      schedule.end(),
      [this, &i] () mutable { return (i++)%num_items; }
    );
  }

  size_t GetScheduleSize() const { return schedule.size(); }
  size_t GetNumItems() const { return num_items; }
  const schedule_t & GetCurSchedule() const { return schedule; }
  const emp::IndexMap & GetWeightMap() const { return weight_map; }

  /// Return a random index where probabilities are weighted according to the weight map.
  size_t GetRandom() {
    const double total_weight = weight_map.GetWeight();
    return weight_map.Index(random.GetDouble() * total_weight);
  }

  /// Update the schedule according to the current weight settings
  const schedule_t & UpdateSchedule() {
    const double total_weight = weight_map.GetWeight();
    emp_assert(total_weight > 0);
    std::generate(
      schedule.begin(),
      schedule.end(),
      [this, &total_weight] () { return weight_map.Index(random.GetDouble() * total_weight); }
    );
    return schedule;
  }

  /// Update the schedule according to the current weight settings, but specify a new schedule size.
  const schedule_t & UpdateSchedule(size_t schedule_size) {
    emp_assert(schedule_size > 0);
    schedule.resize(schedule_size);
    return UpdateSchedule();
  }

  /// Adjust the an item's weight in the weight map
  void AdjustWeight(size_t item_id, double new_weight) {
    weight_map.Adjust(item_id, new_weight);
  }

  /// Give access to defer referesh on weight map
  void DeferWeightRefresh() {
    weight_map.DeferRefresh();
  }

  /// Hard reset on the scheduler
  void Reset(size_t n_items, size_t schedule_size) {
    num_items = n_items;
    schedule.resize(schedule_size);
    size_t i=0;
    std::generate(
      schedule.begin(),
      schedule.end(),
      [this, &i] () mutable { return (i++)%num_items; }
    );
    weight_map.ResizeClear(num_items);
  }

  /// Hard reset on the scheduler
  /// Defaults to schedule size = number of items
  void Reset(size_t n_items) {
    Reset(n_items, n_items); // Default new schedule size to be item count
  }

};

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_PROBABILISTIC_SCHEDULER_HPP_INCLUDE