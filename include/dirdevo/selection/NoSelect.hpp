#pragma once

#include <numeric>

#include "BaseSelect.hpp"

namespace dirdevo {

  /// Performs no selection. I.e., everything is selected.
  /// Assumes n == number of candidates for selection
  struct NoSelect : public BaseSelect {

    emp::vector<size_t>& operator()(size_t n) override {
      selected.resize(n, 0);
      std::iota(
        selected.begin(),
        selected.end(),
        0
      );
      return selected;
    }

  };
}