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

  /// Generates a 100-length genome capable of self-replication.
  // TODO - is the organism the best place for this? Maybe the task should be generating the ancestral genome given that it manages everything?
  template<typename EXPERIMENT_T, typename WORLD_T>
  static genome_t GenerateAncestralGenome(const EXPERIMENT_T& exp, const WORLD_T& world) {
    hardware_t hw(world.GetTask().GetInstLib()); // need this dummy hardware because of the wonky way AvidaGP is implemented
    // TODO - load common ancestor from file!
    hw.PushInst("Scope", 0);
    for (size_t i = 0; i < 94; ++i) {
      hw.PushInst("Nop");
    }
    hw.PushInst("GetLen", 15);
    hw.PushInst("Countdown", 15, 1);
    hw.PushInst("CopyInst", 0);
    hw.PushInst("Scope", 0);
    hw.PushInst("DivideSelf");

    return hw.GetGenome();
  }

  /// Loads an ancestral genome from file.
  template<typename EXPERIMENT_T, typename WORLD_T>
  static genome_t LoadAncestralGenome(const EXPERIMENT_T& exp, const WORLD_T& world) {
    hardware_t hw(world.GetTask().GetInstLib()); // need this dummy hardware because of the wonky way AvidaGP is implemented
    hw.Load(world.GetConfig().ANCESTOR_FILE());
    return hw.GetGenome();
  }

protected:
  // sgp_cpu_t cpu;
  phenotype_t phenotype;
  hardware_t hardware;

  size_t age = 0;
  size_t generation = 0;
  size_t num_pathways = 1;
  size_t cpu_cycles_since_division=0;
  size_t cpu_cycles_per_replication=0;

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

  size_t GetAge() const { return age; }
  void IncGeneration(size_t inc=1) { generation += inc; }
  size_t GetGeneration() const { return generation; }

  size_t GetCPUCyclesPerReplication() const { return cpu_cycles_per_replication; }

  void SetNumPathways(size_t n_pathways) {
    num_pathways=n_pathways;
    hardware.SetNumPathways(n_pathways);
  }

  void OnInjectReady() override {
    hardware.ResetReplicatorHardware();
    dead=false;
    repro_ready=false;
    new_born=true;
    age=0;
    generation=0;
    is_parent=false;
  }

  void OnBeforeRepro() override { }

  void OnOffspringReady(this_t& offspring) override {
    // Reset this (the parent) organism's hardware + reproduction status
    hardware.ResetReplicatorHardware();
    repro_ready=false;
    cpu_cycles_per_replication=cpu_cycles_since_division;
    cpu_cycles_since_division=0;
  }

  void OnPlacement(size_t position) override {
    // let hardware know where it exists in the world
    hardware.SetWorldID(position);
  }

  void OnBirth(this_t& parent) override {
    // note, this happens before parent's OnOffspringReady is called
    hardware.ResetReplicatorHardware(); // Reset AvidaGP virtual hardware
    dead=false;
    repro_ready=false;
    new_born=true;     // TODO - do something with new_born or cut it?
    is_parent=false;
    age=0;
    // when something is born, it is not a parent, so no cpu cycles since division
    cpu_cycles_since_division=0;
    cpu_cycles_per_replication=0;
    // when an organism divides, it and its offspring have an incremented generation
    parent.IncGeneration();
    generation=parent.GetGeneration();
  }

  void OnDeath(size_t position) override { /*TODO*/ }

  template<typename WORLD_T>
  void ProcessStep(WORLD_T& world) {
    // TODO - fill out process step
    // Advance virtual CPU by one step
    hardware.SingleProcess();
    // Is this organism reproducing?
    repro_ready = hardware.IsDividing();
    // Age up
    age+=1;
    cpu_cycles_since_division+=1;
  }

};

}

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_SGP_LITE_ORGANISM_HPP_INCLUDE