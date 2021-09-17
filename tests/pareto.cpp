#define CATCH_CONFIG_MAIN

#include "Catch/single_include/catch2/catch.hpp"

#include <unordered_set>

#include "emp/datastructs/vector_utils.hpp"

#include "dirdevo/utility/pareto.hpp"


TEST_CASE("Test pareto utilities", "[utility][pareto][dominate]")
{

  // No matter where we start, neither a nor b dominate one another
  emp::vector<size_t> a({0,0,0});
  emp::vector<size_t> b({0,0,0});
  for (size_t start = 0; start < a.size(); ++start) {
    CHECK(!dirdevo::dominates(a, b, start));
    CHECK(!dirdevo::dominates(b, a, start));
  }

  // No matter where we start, a dominates b
  a={1,1,1};
  b={0,0,0};
  for (size_t start = 0; start < a.size(); ++start) {
    CHECK(dirdevo::dominates(a, b, start));
    CHECK(!dirdevo::dominates(b, a, start));
  }

  // Neither dominate, unless you start at the final element.
  a={1,0,1};
  b={0,1,0};
  CHECK(!dirdevo::dominates(a, b));
  CHECK(!dirdevo::dominates(b, a));
  CHECK(dirdevo::dominates(a, b, 2));
  CHECK(!dirdevo::dominates(b, a, 2));

  // a dominates b
  a={1,0,1};
  b={1,0,0};
  CHECK(dirdevo::dominates(a, b));
  CHECK(!dirdevo::dominates(b, a));

  // a dominates b (unless you start at the final element)
  a={1,0,1};
  b={0,0,1};
  CHECK(dirdevo::dominates(a, b));
  CHECK(!dirdevo::dominates(b, a));
  CHECK(!dirdevo::dominates(a, b, 2));
  CHECK(!dirdevo::dominates(b, a, 2));

}