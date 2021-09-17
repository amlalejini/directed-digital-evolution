#pragma once

#include <map>
#include <numeric>
#include <algorithm>

#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"

#include "BaseSelect.hpp"
#include "../utility/pareto.hpp"

namespace dirdevo {

/// Multiobjective selection scheme.
/// Find the set of non-dominated candidates (i.e., elites) and select them.
struct NonDominatedEliteSelect : public BaseSelect {
  using score_fun_t = std::function<double(void)>;

  emp::vector< emp::vector<score_fun_t> >& score_fun_sets;
  emp::Random& random;    ///< When front size does not evenly fit into selected, use random to determine which things in the front fill the gap

  emp::vector< emp::vector<double> > score_table;
  // emp::vector<size_t> all_candidates;

  NonDominatedEliteSelect(
    emp::vector< emp::vector<score_fun_t> >& a_score_fun_sets,
    emp::Random& a_random
  ) :
    score_fun_sets(a_score_fun_sets),
    random(a_random)
  {  }

  emp::vector<size_t>& operator()(size_t n) override {
    selected.resize(n, 0);
    const size_t num_candidates = score_fun_sets.size();
    emp_assert(num_candidates > 0);
    const size_t obj_cnt = score_fun_sets[0].size();

    // update the score table
    score_table.resize(num_candidates);
    for (size_t cand_i = 0; cand_i < num_candidates; ++cand_i) {
      score_table[cand_i].resize(obj_cnt);
      for (size_t fun_i = 0; fun_i < obj_cnt; ++fun_i) {
        emp_assert(obj_cnt == score_fun_sets[cand_i].size());
        score_table[cand_i][fun_i] = score_fun_sets[cand_i][fun_i]();
      }
    }

    // find the pareto front
    emp::vector<size_t> front(dirdevo::find_pareto_front(score_table));
    emp::Shuffle(random, front);
    const size_t front_size = front.size();
    // select the front
    for (size_t i = 0; i < n; ++i) {
      selected[i] = front[i % front_size];
    }

    return selected;
  }

};

} // namespace dirdevo
