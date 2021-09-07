#pragma once
#ifndef DIRECTED_DEVO_SELECTION_DIRECTED_DEVO_ELITE_HPP_INCLUDE
#define DIRECTED_DEVO_SELECTION_DIRECTED_DEVO_ELITE_HPP_INCLUDE

#include <map>

#include "emp/base/vector.hpp"

// TODO - convert this into struct w/() operator overloads if we want to track state information during selection?
//      - or if we need to configure the selection scheme up front and then;
//      problem with struct: requires you to hold on to it, so you need to know its type => Experiment needs one of each selection type that it knows about (which is probably fine)

namespace dirdevo {

/// ==ELITE== Selection picks a set of the most fit individuals from the population to move to
/// the next generation.  Find top e_count individuals and make copy_count copies of each.
/// @param e_count How many distinct organisms should be chosen, starting from the most fit.
/// @param copy_count How many copies should be made of each elite organism?
/// @return vector of selected ids (indices into scores)
emp::vector<size_t> EliteSelect(const emp::vector<double>& scores, size_t e_count=1, size_t copy_count=1) {
  emp_assert(e_count > 0, e_count);
  emp_assert(e_count <= scores.size(), e_count, scores.size());
  emp_assert(copy_count > 0, copy_count);

  emp::vector<size_t> selected(e_count*copy_count, 0);

  std::multimap<double, size_t> fit_map;
  for (size_t id = 0; id < scores.size(); ++id) {
    fit_map.insert( std::make_pair(scores[id], id) );
  }

  // Grab the top fitnesses and move them into selected.
  auto m = fit_map.rbegin();
  for (size_t i = 0; i < e_count; ++i) {
    const size_t repro_id = m->second;
    for (size_t c=0; c < copy_count; ++c) {
      const size_t selected_idx = (i*copy_count)+c;
      selected[selected_idx] = repro_id;
    }
    ++m;
  }

  return selected;
}


void EliteSelect(emp::vector<size_t> & selected, const emp::vector<std::function<double(void)>>& score_funs, size_t e_count=1) {
  emp_assert(e_count > 0, e_count);
  emp_assert(e_count <= score_funs.size(), e_count, score_funs.size());

  // sort fitnesses
  std::multimap<double, size_t> fit_map;
  // for (size_t id = 0; id < scores.size(); ++id) {
  for (size_t id = 0; id < score_funs.size(); ++id) {
    fit_map.insert( std::make_pair(score_funs[id](), id) );
  }

  // Grab the top fitnesses and move them into selected.
  auto m = fit_map.rbegin();
  emp::vector<size_t> elite_ids(e_count, 0);
  for (size_t i = 0; i < e_count; ++i) {
    const size_t repro_id = m->second;
    elite_ids[i] = repro_id;
  }

  // fill selected with elites
  for (size_t i = 0; i < selected.size(); ++i) {
    selected[i] = elite_ids[i % e_count];
  }
}

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_SELECTION_DIRECTED_DEVO_ELITE_HPP_INCLUDE