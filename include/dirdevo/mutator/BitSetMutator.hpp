#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_BITSET_MUTATOR_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_BITSET_MUTATOR_HPP_INCLUDE

#include <algorithm>
#include <numeric>

#include "emp/bits/BitSet.hpp"
#include "emp/config/config.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

namespace dirdevo {

/// A minimal BitSet Mutator
/// TODO - define a base mutator that enforces that certain funtions exist!
class BitSetMutator {
public:

  /// Describes the configuration
  struct MutatorConfig {
    double PER_SITE_SUBSTITUTION_RATE=0.0; /// Per-bit bitflip rate

  };

  using mut_config_t = MutatorConfig;
  using this_t = BitSetMutator;

  static void Configure(this_t& mutator, const emp::Config& exp_config) {

    // From the experiment configuration, configure the mutator.
    const std::string per_site_substitution_name(mutator.config_prepend+"_"+"PER_SITE_SUBSTITUTION_RATE");
    emp_assert(exp_config.Has(per_site_substitution_name), "Failed to find parameter ", per_site_substitution_name, " in experiment configuration.");
    mutator.config.PER_SITE_SUBSTITUTION_RATE = emp::from_string<double>(exp_config[per_site_substitution_name]->GetValue());

    std::cout << "per site sub: " << mutator.config.PER_SITE_SUBSTITUTION_RATE << std::endl;
  }

protected:

  mut_config_t config;
  std::string config_prepend;

public:
  BitSetMutator()
    : config_prepend("BITSET_MUTATOR") // TODO - clean this up!
  { ; }

  // TODO - enable more sophisticated mutation tracking
  template<size_t LEN>
  size_t Mutate(emp::BitSet<LEN>& bits, emp::Random& random) {
    size_t flips = 0;
    for (size_t i = 0; i < LEN; ++i) {
      if (random.P(config.PER_SITE_SUBSTITUTION_RATE)) {
        bits.Toggle(i);
        ++flips;
      }
    }
    return flips;
  }

};

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_BITSET_MUTATOR_HPP_INCLUDE