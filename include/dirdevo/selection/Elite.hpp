#pragma once
#ifndef DIRECTED_DEVO_SELECTION_DIRECTED_DEVO_ELITE_HPP_INCLUDE
#define DIRECTED_DEVO_SELECTION_DIRECTED_DEVO_ELITE_HPP_INCLUDE

#include <map>

#include "emp/base/vector.hpp"

#include "BaseSelect.hpp"

namespace dirdevo {

struct EliteSelect : public BaseSelect {
  using score_fun_t = std::function<double(void)>;

  emp::vector<score_fun_t>& score_funs; ///< One function for each selection candidate (e.g., each member of the population)
  size_t elite_count;                  ///< How many distinct candidates should be chosen (in rank order by score)

  EliteSelect(
    emp::vector<score_fun_t>& a_score_funs,
    size_t a_elite_count=1
  ) :
    score_funs(a_score_funs),
    elite_count(a_elite_count)
  { }

  emp::vector<size_t>& operator()(size_t n) override {
    emp_assert(elite_count <= score_funs.size(), elite_count, score_funs.size());

    selected.resize(n, 0);

    const size_t num_candidates = score_funs.size();

    std::multimap<double, size_t> fit_map;
    for (size_t id = 0; id < num_candidates; ++id) {
      fit_map.insert(
        std::make_pair(score_funs[id](), id)
      );
    }

    // Identify the elites
    emp::vector<size_t> elites(elite_count, 0);
    auto m = fit_map.rbegin();
    for (size_t i = 0; i < elite_count; ++i) {
      elites[i] = m->second;
      ++m;
    }
    // Fill selected with elites
    for (size_t i = 0; i < n; ++i) {
      selected[i] = elites[i % elite_count];
    }

    return selected;
  }

};

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_SELECTION_DIRECTED_DEVO_ELITE_HPP_INCLUDE