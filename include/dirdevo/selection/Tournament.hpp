#pragma once

#include <functional>
#include <algorithm>

#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"

#include "BaseSelect.hpp"

namespace dirdevo {


struct TournamentSelect : public BaseSelect {

  using score_fun_t = std::function<double(void)>;

  emp::Random& random;
  emp::vector<score_fun_t>& score_funs;     ///< One function for each selection candidate (e.g., each member of the population)
  size_t tournament_size;

  TournamentSelect(
    emp::Random& a_random,
    emp::vector<score_fun_t>& a_score_funs,
    size_t a_tournament_size=4
  ) :
    random(a_random),
    score_funs(a_score_funs),
    tournament_size(a_tournament_size)
  { }

  emp::vector<size_t>& operator()(size_t n) override {
    emp_assert(tournament_size > 0, "Tournament size must be greater than 0.", tournament_size);
    emp_assert(tournament_size <= score_funs.size(), "Tournament size should not exceed number of individuals that we can select from.", tournament_size, score_funs.size());

    const size_t num_candidates = score_funs.size();

    selected.resize(n, 0); // Update size of selected
    std::fill(
      selected.begin(),
      selected.end(),
      0
    );

    emp::vector<size_t> entries(tournament_size, 0); // Used to keep track of tournament entries for each tournament

    for (size_t t=0; t < n; ++t) {
      // form a tournament
      std::generate(
        entries.begin(),
        entries.end(),
        [this, num_candidates](){ return random.GetUInt(num_candidates); }
      );
      // pick a winner
      size_t winner_id = entries[0];
      double winner_fit = score_funs[entries[0]]();
      for (size_t i = 1; i < entries.size(); ++i) {
        const size_t entry_id = entries[i];
        const double entry_fit = score_funs[entry_id]();
        if (entry_fit > winner_fit) {
          winner_id = entry_id;
        }
      }
      // save winner of tournament t
      selected[t] = winner_id;
    }
    return selected; // return selected for convenience
  }

};

}