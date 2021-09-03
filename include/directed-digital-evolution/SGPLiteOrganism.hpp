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

// STATUS: In progress

namespace dirdevo {

class SGPLiteOrganism : public BaseOrganism<SGPLiteOrganism> {

public:
  // struct Genome;
  struct Phenotype;

  using this_t = SGPLiteOrganism;
  using base_t = BaseOrganism<this_t>;
  using sgp_library_t = sgpl::OpLibraryCoupler<sgpl::ArithmeticOpLibrary>;
  using sgp_spec_t = sgpl::Spec<sgp_library_t>;
  using sgp_program_t = sgpl::Program<sgp_spec_t>;
  using sgp_cpu_t = sgpl::Cpu<sgp_spec_t>;

  using genome_t = sgp_program_t;
  using phenotype_t = Phenotype;

protected:
  sgp_cpu_t cpu;
  genome_t genome;
  // sgp_program_t program;

public:

  SGPLiteOrganism(const genome_t& g) :
    genome(g)
  {

  }

  void OnBeforeRepro() override;

  void OnOffspringReady(this_t& offspring) override;

  void OnPlacement(size_t position) override;

  void OnBirth(this_t& parent) override {
    // After mutations have occurred, but before parent & task have been alerted to ready-ness.
    // Safe to spin up the CPU with the current program at this point.
    cpu.InitializeAnchors(genome);
    //
  }

  void OnDeath(size_t position) override;

  template<typename WORLD_T>
  void ProcessStep(WORLD_T& world) {

  }

};

}

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_SGP_LITE_ORGANISM_HPP_INCLUDE