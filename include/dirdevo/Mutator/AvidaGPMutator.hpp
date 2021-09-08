#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_MUTATOR_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_MUTATOR_HPP_INCLUDE

#include <algorithm>
#include <numeric>

#include "emp/config/config.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/hardware/AvidaGP.hpp"
#include "emp/math/Range.hpp"

#include "dirdevo/AvidaGPL9/AvidaGPReplicator.hpp"

namespace dirdevo {

class AvidaGPMutator {
public:

  using this_t = AvidaGPMutator;
  using genome_t = typename AvidaGPReplicator::genome_t;
  static constexpr size_t INST_ARGS = AvidaGPReplicator::hardware_t::INST_ARGS;
  static constexpr size_t CPU_SIZE = AvidaGPReplicator::hardware_t::CPU_SIZE;

  static void Configure(this_t& mutator, const emp::Config& exp_config) {
    // From the experiment configuration, configure the mutator.

    emp_assert(exp_config.Has("AVIDAGP_MUT_RATE_INST_SUB"), "Failed to find parameter in experiment configuration.");
    mutator.rate_inst_substitution = emp::from_string<double>(
      exp_config["AVIDAGP_MUT_RATE_INST_SUB"]->GetValue()
    );

    emp_assert(exp_config.Has("AVIDAGP_MUT_RATE_ARG_SUB"), "Failed to find parameter in experiment configuration.");
    mutator.rate_arg_substitution = emp::from_string<double>(
      exp_config["AVIDAGP_MUT_RATE_ARG_SUB"]->GetValue()
    );

  }

protected:

  // Per-site substitutions
  double rate_inst_substitution=0;
  double rate_arg_substitution=0;

public:

  // TODO - enable more sophisticated mutation tracking
  size_t Mutate(genome_t& genome, emp::Random& random) {
    // TODO - implement single instruction deletion/mutation?
    size_t count=0;
    const size_t inst_lib_size = genome.GetInstLib()->GetSize();
    for (size_t inst_i = 0; inst_i < genome.GetSize(); ++inst_i) {
      // Mutate instruction operation
      if (random.P(rate_inst_substitution)) {
        genome[inst_i].id = random.GetUInt(inst_lib_size);
        ++count;
      }
      // Mutate arguments
      for (size_t arg_i = 0; arg_i < INST_ARGS; ++arg_i) {
        if (random.P(rate_arg_substitution)) {
          genome[inst_i].args[arg_i] = random.GetUInt(CPU_SIZE);
          ++count;
        }
      }
    }
    return count;
  }


};

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_MUTATOR_HPP_INCLUDE