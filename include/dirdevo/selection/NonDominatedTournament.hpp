#pragma once

#include <functional>
#include <algorithm>

#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"

#include "BaseSelect.hpp"
#include "../utility/pareto.hpp"

namespace dirdevo {

struct NonDominatedTournamentSelect : public BaseSelect {
  using score_fun_t = std::function<double(void)>;

  emp::vector< emp::vector<score_fun_t> >& score_fun_sets;
  emp::Random& random;    ///< When front size does not evenly fit into selected, use random to determine which things in the front fill the gap
  size_t tournament_size;

  emp::vector< emp::vector<double> > score_table;

  NonDominatedTournamentSelect(
    emp::vector< emp::vector<score_fun_t> >& a_score_fun_sets,
    emp::Random& a_random,
    size_t a_tournament_size
  ) :
    score_fun_sets(a_score_fun_sets),
    random(a_random),
    tournament_size(a_tournament_size)
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

    // the entries for each tournament
    emp::vector<size_t> entries(tournament_size);
    emp::vector< emp::vector<double> > entries_score_table(tournament_size, emp::vector<double>(obj_cnt));
    // run enough tournaments to get n winners
    size_t winners = 0;
    while (winners < n) {
      // form a tournament
      std::generate(
        entries.begin(),
        entries.end(),
        [this, num_candidates]() { return random.GetUInt(num_candidates); }
      );
      // copy score vectors of entrants into the tournament entry score table
      for (size_t i = 0; i < tournament_size; ++i) {
        const size_t entry_id = entries[i];
        std::copy(
          entries_score_table[i].begin(),
          entries_score_table[i].end(),
          score_table[entry_id].begin()
        );
        emp_assert(score_table[entry_id] == entries_score_table[i], score_table, entries_score_table);
      }

      // compute the pareto front of the entries
      emp::vector<size_t> tournament_front(dirdevo::find_pareto_front(entries_score_table));

      // select the winner(s), until we've selected enough things or until we've selected everything from this tournament
      for (size_t i = 0; i < tournament_front.size() && winners < selected.size(); ++i) {
        const size_t winner_id = entries[tournament_front[i]];
        selected[winners] = winner_id;
        ++winners;
      }
    }
    return selected;
  }


};

}