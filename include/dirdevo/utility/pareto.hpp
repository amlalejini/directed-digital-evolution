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

}