#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_SGP_LITE_ORGANISM_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_SGP_LITE_ORGANISM_HPP_INCLUDE

#include "BaseOrganism.hpp"
#include "sgpl/algorithm/execute_cpu.hpp"
#include "sgpl/spec/Spec.hpp"
#include "sgpl/hardware/Cpu.hpp"
#include "sgpl/library/OpLibraryCoupler.hpp"
#include "sgpl/library/prefab/prefab.hpp"
#include "sgpl/program/Program.hpp"

namespace dirdevo {

class SGPLiteOrganism : public BaseOrganism<SGPLiteOrganism> {

public:
  struct Genome;
  struct Phenotype;

  using this_t = SGPLiteOrganism;
  using base_t = BaseOrganism<this_t>;
  using sgp_library_t = sgpl::OpLibraryCoupler<sgpl::ArithmeticOpLibrary>;
  using sgp_spec_t = sgpl::Spec<sgp_library_t>;

  using genome_t = Genome;
  using phenotype_t = Phenotype;

protected:
  sgpl::Cpu<sgp_spec_t> cpu;
  sgpl::Program<sgp_spec_t> program;

public:
  // SGPLiteOrganism()


};

}

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_SGP_LITE_ORGANISM_HPP_INCLUDE