#define CATCH_CONFIG_MAIN

#include "Catch/single_include/catch2/catch.hpp"

#include <unordered_set>

#include "emp/datastructs/vector_utils.hpp"

#include "dirdevo/selection/Elite.hpp"
#include "dirdevo/selection/Lexicase.hpp"
#include "dirdevo/selection/NonDominatedElite.hpp"
#include "dirdevo/selection/Tournament.hpp"
#include "dirdevo/selection/Roulette.hpp"

TEST_CASE("Test Elite Selection", "[selection][elite]")
{
  // Create some scores
  emp::vector<double> scores = {/*0:*/ 8, /*1:*/ 128, /*2:*/ 2, /*3:*/ 32, /*4:*/ 16, /*5:*/ 4};
  emp::vector<size_t> results;
  std::unordered_set<size_t> results_set;

  SECTION("emp::vector<size_t> EliteSelect(const emp::vector<double>&, size_t, size_t)") {

    results = dirdevo::EliteSelect(scores, 1, 1);
    CHECK(results==emp::vector<size_t>({1}));

    results = dirdevo::EliteSelect(scores, 1, 100);
    CHECK(results==emp::vector<size_t>(100, 1));

    results = dirdevo::EliteSelect(scores, 3, 1);
    CHECK(results.size()==3);
    CHECK(emp::Has(results, (size_t)1));
    CHECK(emp::Has(results, (size_t)3));
    CHECK(emp::Has(results, (size_t)4));

    results = dirdevo::EliteSelect(scores, 3, 10);
    CHECK(results.size()==30);
    results_set.clear();
    results_set.insert(results.begin(), results.end());
    CHECK(results_set.size()==3);
    CHECK(results_set==std::unordered_set<size_t>({1,3,4}));

    results = dirdevo::EliteSelect(scores, 5, 2);
    CHECK(results.size()==10);
    results_set.clear();
    results_set.insert(results.begin(), results.end());
    CHECK(results_set.size()==5);
    CHECK(results_set==std::unordered_set<size_t>({1,3,4,0,5}));
    CHECK(emp::Count(results, (size_t)1)==2);

  }

  SECTION("void EliteSelect(emp::vector<size_t>&, const emp::vector<double>&, size_t)") {
    // TODO
  }

}
