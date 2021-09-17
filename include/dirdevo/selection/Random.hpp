#pragma once

#include <algorithm>

#include "emp/math/Random.hpp"

#include "BaseSelect.hpp"

namespace dirdevo {
  struct RandomSelect : public BaseSelect {

    emp::Random& random;
    size_t num_candidates;

    RandomSelect(
      emp::Random& a_random,
      size_t a_num_candidates
    ) :
      random(a_random),
      num_candidates(a_num_candidates)
    { }

    emp::vector<size_t>& operator()(size_t n) override {
      emp_assert(num_candidates > 0);
      selected.resize(n, 0);
      std::generate(
        selected.begin(),
        selected.end(),
        [this]() { return random.GetUInt(num_candidates); }
      );
      return selected;
    }

  };
}