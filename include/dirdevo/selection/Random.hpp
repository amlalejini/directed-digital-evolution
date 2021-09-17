#pragma once

#include <functional>
#include <algorithm>

#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"

#include "BaseSelect.hpp"

namespace dirdevo {
  struct RandomSelect : public BaseSelect {

    emp::Random& random;

    RandomSelect(
      emp::Random& a_random
    ) :
      random(a_random)
    { }

    emp::vector<size_t>& operator()(size_t n) override {
      selected.resize(n, 0);
      std::generate(
        selected.begin(),
        selected.end(),
        [this]() { return random.GetUInt(selected.size()); }
      );
      return selected;
    }

  };
}