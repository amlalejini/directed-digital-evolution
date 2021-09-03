#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_ORGANISM_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_ORGANISM_HPP_INCLUDE

#include "../BaseOrganism.hpp"
#include "emp/hardware/AvidaGP.hpp"

// STATUS: In progress

// NOTE - Empirical's AvidaGP has some ...interesting... design decisions in terms of what classes own
//        AvidaGPOrganism is NOT necessarily a good example of how to setup organisms.

namespace dirdevo {

class AvidaGPOrganism : public BaseOrganism<AvidaGPOrganism> {

public:
  struct Genome;
  struct Phenotype;

  using this_t = AvidaGPOrganism;
  using base_t = BaseOrganism<this_t>;
  using hardware_t = emp::AvidaGP;
  using genome_t = typename hardware_t::genome_t;
  using phenotype_t = Phenotype;

  struct Phenotype {

  };

  template<typename EXPERIMENT_T, typename WORLD_T>
  static genome_t GenerateAncestralGenome(const EXPERIMENT_T& exp, const WORLD_T& world) {
    hardware_t hw(world.GetInstLib()); // need this dummy hardware because of the wonky way AvidaGP is implemented
    // TODO - load common ancestor from file! For now, just push some nops.
    for (size_t i = 0; i < 100; ++i) {
      hw.PushInst("Nop");
    }
    return hw.GetGenome();
  }

protected:
  // sgp_cpu_t cpu;
  phenotype_t phenotype;
  hardware_t hardware;
  // genome_t genome;      /// Keep our own copy of our genome independent of the hardware? (can't easily mutate the genome in the hardware)
  // sgp_program_t program;

public:

  AvidaGPOrganism(const genome_t& g) :
    hardware(g)
  {

  }

  genome_t & GetGenome() { return hardware.genome; }
  const genome_t & GetGenome() const { return hardware.genome; }
  phenotype_t & GetPhenotype() { return phenotype; }
  const phenotype_t & GetPhenotype() const { return phenotype; }


  void OnBeforeRepro() override;

  void OnOffspringReady(this_t& offspring) override;

  void OnPlacement(size_t position) override;

  void OnBirth(this_t& parent) override {
    // After mutations have occurred, but before parent & task have been alerted to ready-ness.
    // Safe to spin up the CPU with the current program at this point.
    // cpu.InitializeAnchors(genome);
    //
  }

  void OnDeath(size_t position) override;

  template<typename WORLD_T>
  void ProcessStep(WORLD_T& world) {

  }

};

}

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_SGP_LITE_ORGANISM_HPP_INCLUDE