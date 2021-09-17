#pragma once

#include "emp/base/vector.hpp"

namespace dirdevo {

  /// Does a dominate b?
  template<typename T>
  bool dominates(const emp::vector<T>& a, const emp::vector<T>& b, size_t start=0) {
    emp_assert(a.size() == b.size(), "a and b should be the same size.", a.size(), b.size());
    emp_assert(start < a.size(), start, a.size(), b.size());
    bool equal = true;
    for (size_t i = start; i < a.size(); ++i) {
      // if a is ever less than b, it does not dominate
      if (a[i] < b[i]) return false;
      else if (a[i] > b[i]) equal = false;
    }
    return !equal; // if a and b are not equal, a must dominate b (if b were ever greater than a, we would have returned false already)
  }

    /// Each vector<double> in score table represents a single candidate's scores on a set of goals/objectives
  emp::vector<size_t> find_pareto_front(const emp::vector< emp::vector<double> >& score_table) {

    const size_t num_candidates = score_table.size();

    emp::vector<size_t> all_candidates(num_candidates);
    std::iota(
      all_candidates.begin(),
      all_candidates.end(),
      0
    );

    // -- Find the pareto front --
    // Initialize the front with the first candidate
    emp::vector<size_t> front({all_candidates[0]});
    for (size_t cand_i = 1; cand_i < all_candidates.size(); ++cand_i) {
      const size_t cur_candidate_id = all_candidates[cand_i];
      bool dominated = false; // tracks whether this candidate has been dominated by anything on the front
      for (int front_i = 0; front_i < (int)front.size(); ++front_i) {
        const size_t front_id = front[front_i];
        emp_assert(score_table[cur_candidate_id].size() == score_table[front_id].size());
        // note, this could be made more efficient (by computing sharing domination comparisons)
        if (dirdevo::dominates(score_table[cur_candidate_id], score_table[front_id])) {
          // candidate dominates member of the front, remove member of the front
          std::swap(front[front_i], front[front.size()-1]); // Put dominated member of the front in the back
          front.pop_back(); // Remove dominated member of the front
          --front_i; // need to look at this index again
        } else if (dirdevo::dominates(score_table[front_id], score_table[cur_candidate_id])) {
          dominated = true;
          break;
        }
      }
      if (!dominated) front.emplace_back(cur_candidate_id);
    }

    return front;
  }


}