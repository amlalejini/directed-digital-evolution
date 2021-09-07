#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_MUTATOR_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_MUTATOR_HPP_INCLUDE

#include <algorithm>
#include <numeric>

#include "emp/config/config.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

#include "emp/hardware/AvidaGP.hpp"

namespace dirdevo {

/// TODO - define a base mutator that enforces that certain funtions exist!
class AvidaGPMutator {
public:

  /// Describes the configuration
  struct MutatorConfig {
    double PER_SITE_SUBSTITUTION_RATE=0.0; /// Per-bit bitflip rate
  };

  using mut_config_t = MutatorConfig;
  using this_t = AvidaGPMutator;
  using genome_t = typename emp::AvidaGP::genome_t;

  static void Configure(this_t& mutator, const emp::Config& exp_config) {

    // From the experiment configuration, configure the mutator.
    // const std::string per_site_substitution_name(mutator.config_prepend+"_"+"PER_SITE_SUBSTITUTION_RATE");
    // emp_assert(exp_config.Has(per_site_substitution_name), "Failed to find parameter ", per_site_substitution_name, " in experiment configuration.");
    // mutator.config.PER_SITE_SUBSTITUTION_RATE = emp::from_string<double>(exp_config[per_site_substitution_name]->GetValue());

    // std::cout << "per site sub: " << mutator.config.PER_SITE_SUBSTITUTION_RATE << std::endl;
  }

protected:

  mut_config_t config;
  std::string config_prepend;

public:
  AvidaGPMutator() :
    config_prepend("AVIDAGP_MUTATOR") // TODO - clean this up!
  { ; }

  // TODO - enable more sophisticated mutation tracking
  size_t Mutate(genome_t& genome, emp::Random& random) {
    // TODO - implement AvidaGP mutation.
    return 0;
  }

};

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_MUTATOR_HPP_INCLUDE