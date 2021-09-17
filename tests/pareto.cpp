#define CATCH_CONFIG_MAIN

#include "Catch/single_include/catch2/catch.hpp"

#include <unordered_set>

#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/datastructs/vector_utils.hpp"

#include "dirdevo/utility/pareto.hpp"


TEST_CASE("Test dirdevo::dominates", "[utility][pareto][dominates]")
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

TEST_CASE("Test dirdevo::find_pareto_front", "[utility][pareto][find_pareto_front]") {
  using set_t = std::unordered_set<size_t>;

  emp::Random random(2);

  // everything is part of the front
  emp::vector< emp::vector<double> > score_table({
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
  });
  emp::vector<size_t> front = dirdevo::find_pareto_front(score_table);
  CHECK(front.size() == 3);
  CHECK(set_t(front.begin(), front.end()) == set_t({0,1,2}));

  // 1 dominates all
  score_table = {
    {1, 0, 0},
    {1, 1, 1},
    {0, 0, 1},
  };
  front = dirdevo::find_pareto_front(score_table);
  CHECK(front.size() == 1);
  CHECK(set_t(front.begin(), front.end()) == set_t({1}));

  // everything is part of the front
  score_table = {
    {1, 0},
    {0, 1},
    {0.75, 0.75},
    {0.5, 0.8},
    {0.8, 0.5},
    {0.1, 0.9},
    {0.9, 0.05},
    {1, 0}
  };
  emp::Shuffle(random, score_table);
  front = dirdevo::find_pareto_front(score_table);
  CHECK(front.size() == score_table.size());
  CHECK(set_t(front.begin(), front.end()) == set_t({0,1,2,3,4,5,6,7}));

  // emp::vector< emp::vector<double> > correct_front_values(score_table);
  // create a bunch of dominated values and add to the score vector
  size_t trials = 100;
  size_t num_extra_values = 10000;
  for (size_t trial_i = 0; trial_i < trials; ++trial_i) {
    emp::vector< emp::vector<double> > trial_score_table(score_table);
    for (size_t val_i = 0; val_i < num_extra_values; ++val_i) {
      // Pick a random obj vector
      size_t front_id = random.GetUInt(score_table.size());
      double obj0 = score_table[front_id][0] - random.GetDouble();
      double obj1 = score_table[front_id][1] - random.GetDouble();
      trial_score_table.emplace_back(emp::vector<double>({obj0, obj1}));
    }
    emp::Shuffle(random, trial_score_table);
    auto trial_front = dirdevo::find_pareto_front(trial_score_table);
    emp::vector< emp::vector<double> > trial_front_scores;
    for (size_t id : trial_front) {
      trial_front_scores.emplace_back(
        trial_score_table[id]
      );
    }
    CHECK(trial_front.size() == score_table.size());
    for (const auto& scores : score_table) {
      CHECK(emp::Has(trial_front_scores, scores));
    }
  }

}