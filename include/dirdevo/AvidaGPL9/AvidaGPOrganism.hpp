#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_ORGANISM_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_ORGANISM_HPP_INCLUDE

#include "emp/hardware/Genome.hpp"
#include "emp/hardware/AvidaGP.hpp"

#include "AvidaGPReplicator.hpp"

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

  using hardware_t = AvidaGPReplicator;
  using genome_t = typename AvidaGPReplicator::genome_t;
  using phenotype_t = Phenotype;

  using base_t::SetReproReady;
  using base_t::SetDead;

  struct Phenotype {
    // TODO - manage organism task performance
    emp::vector<size_t> org_task_performances;

    void Reset(size_t num_org_tasks=0) {
      // Reset task performance information for this organism.
      org_task_performances.resize(num_org_tasks);
      std::fill(
        org_task_performances.begin(),
        org_task_performances.end(),
        0
      );
    }

  };

  template<typename EXPERIMENT_T, typename WORLD_T>
  static genome_t GenerateAncestralGenome(const EXPERIMENT_T& exp, const WORLD_T& world) {
    hardware_t hw(world.GetTask().GetInstLib()); // need this dummy hardware because of the wonky way AvidaGP is implemented
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

  size_t age = 0;

  using base_t::dead;
  using base_t::repro_ready;
  using base_t::new_born;

public:

  AvidaGPOrganism(const genome_t& g)
    : hardware(g)
  {

  }

  genome_t & GetGenome() { return hardware.genome; }
  const genome_t & GetGenome() const { return hardware.genome; }
  phenotype_t & GetPhenotype() { return phenotype; }
  const phenotype_t & GetPhenotype() const { return phenotype; }

  hardware_t& GetHardware() { return hardware; }
  const hardware_t& GetHardware() const { return hardware; }


  void OnBeforeRepro() override { }

  void OnOffspringReady(this_t& offspring) override { }
  // TODO - reset parent!

  void OnPlacement(size_t position) override {
    // let hardware know where it exists in the world
    hardware.SetWorldID(position);
  }

  void OnBirth(this_t& parent) override {
    hardware.ResetReplicatorHardware(); // Reset AvidaGP virtual hardware
    dead=false;
    repro_ready=false;
    new_born=true;
    // TODO - reset phenotype
  }

  void OnDeath(size_t position) override { /*TODO*/ }

  template<typename WORLD_T>
  void ProcessStep(WORLD_T& world) {
    // TODO - fill out process step
    // Advance virtual CPU by one step
    hardware.SingleProcess();
    // Is this organism reproducing?
    repro_ready = hardware.IsDividing();
    // TODO - check death condition
  }

};

}

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_SGP_LITE_ORGANISM_HPP_INCLUDE