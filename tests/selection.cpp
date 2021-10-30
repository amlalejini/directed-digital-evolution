#define CATCH_CONFIG_MAIN

#include "Catch/single_include/catch2/catch.hpp"

#include <unordered_set>

#include "emp/datastructs/vector_utils.hpp"

#include "dirdevo/selection/Elite.hpp"
#include "dirdevo/selection/Lexicase.hpp"
#include "dirdevo/selection/NonDominatedElite.hpp"
#include "dirdevo/selection/Tournament.hpp"
#include "dirdevo/selection/NonDominatedTournament.hpp"

TEST_CASE("Test NonDominatedTournamentSelection", "[selection][ndt]") {
  using score_fun_t = std::function<double(void)>;

  constexpr size_t seed = 2;

  emp::vector< emp::vector<score_fun_t> > score_fun_sets;
  emp::vector< emp::vector<double> > scores{
    /* 0= */ {1.0, 1.0, 1.0},
    /* 1= */ {1.0, 0.0, 0.0},
    /* 2= */ {0.0, 1.0, 0.0},
    /* 3= */ {0.0, 0.0, 1.0},
    /* 4= */ {0.0, 0.0, 0.0},
    /* 5= */ {0.0, 2.0, 0.0},
    /* 6= */ {2.0, 0.0, 0.0},
    /* 7= */ {0.0, 0.0, 2.0}
  };
  for (size_t i = 0; i < scores.size(); ++i) {
    score_fun_sets.emplace_back();
    for (size_t j = 0; j < scores[i].size(); ++j) {
      score_fun_sets[i].emplace_back(
        [i,j,&scores](){return scores[i][j];}
      );
    }
  }

  emp::Random random(seed);

  dirdevo::NonDominatedTournamentSelect ndt_4(
    score_fun_sets,
    random,
    8
  );

  std::cout << ndt_4(3) << std::endl;


}

// TEST_CASE("Test Elite Selection", "[selection][elite]")
// {
//   // Create some scores
//   emp::vector<double> scores = {/*0:*/ 8, /*1:*/ 128, /*2:*/ 2, /*3:*/ 32, /*4:*/ 16, /*5:*/ 4};
//   emp::vector<size_t> results;
//   std::unordered_set<size_t> results_set;

//   SECTION("emp::vector<size_t> EliteSelect(const emp::vector<double>&, size_t, size_t)") {

//     results = dirdevo::EliteSelect(scores, 1, 1);
//     CHECK(results==emp::vector<size_t>({1}));

//     results = dirdevo::EliteSelect(scores, 1, 100);
//     CHECK(results==emp::vector<size_t>(100, 1));

//     results = dirdevo::EliteSelect(scores, 3, 1);
//     CHECK(results.size()==3);
//     CHECK(emp::Has(results, (size_t)1));
//     CHECK(emp::Has(results, (size_t)3));
//     CHECK(emp::Has(results, (size_t)4));

//     results = dirdevo::EliteSelect(scores, 3, 10);
//     CHECK(results.size()==30);
//     results_set.clear();
//     results_set.insert(results.begin(), results.end());
//     CHECK(results_set.size()==3);
//     CHECK(results_set==std::unordered_set<size_t>({1,3,4}));

//     results = dirdevo::EliteSelect(scores, 5, 2);
//     CHECK(results.size()==10);
//     results_set.clear();
//     results_set.insert(results.begin(), results.end());
//     CHECK(results_set.size()==5);
//     CHECK(results_set==std::unordered_set<size_t>({1,3,4,0,5}));
//     CHECK(emp::Count(results, (size_t)1)==2);

//   }

//   SECTION("void EliteSelect(emp::vector<size_t>&, const emp::vector<double>&, size_t)") {
//     // TODO
//   }

// }
