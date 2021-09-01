#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONEMAX_ORGANISM_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_ONEMAX_ORGANISM_HPP_INCLUDE

#include "BaseOrganism.hpp"
// #include "BitSetMutator.hpp"

namespace dirdevo {

template<size_t BITS=128>
class OneMaxOrganism : public BaseOrganism<OneMaxOrganism<BITS>> {
public:
  struct Phenotype;
  static constexpr size_t GENOME_SIZE=BITS;
  using genome_t = emp::BitSet<GENOME_SIZE>;
  using phenotype_t = Phenotype;
  using this_t = OneMaxOrganism<GENOME_SIZE>;
  using base_t = BaseOrganism<this_t>;
  // using mutator_t = MUTATOR_T;

  using base_t::SetReproReady;
  using base_t::SetDead;

  const double REPRO_RES_THRESHOLD=1024;
  const size_t MAX_AGE=2048;

  struct Phenotype {
    size_t num_ones=0;
  };

  static genome_t GenerateAncestralGenome() {
    return genome_t(false);
  }

protected:
  genome_t genome;
  phenotype_t phenotype;

  double resources=0;
  size_t repro_count=0; // Number of times this organism has reproduced.
  size_t age=0;

  using base_t::merit;

  double UpdateMerit() {
    merit = 1 + emp::Pow2(phenotype.num_ones); // +1 ensures that merit can't be zero for living organisms
    return merit;
  }


public:
  OneMaxOrganism(const genome_t& g) :
    genome(g)
  {
    phenotype.num_ones = genome.CountOnes();
  }

  double GetResources() const { return resources; }
  genome_t & GetGenome() { return genome; }
  const genome_t & GetGenome() const { return genome; }
  phenotype_t & GetPhenotype() { return phenotype; }
  const phenotype_t & GetPhenotype() const { return phenotype; }

  // todo - make a virtual base function?

  void OnBeforeRepro() override { }

  void OnOffspringReady(this_t & offspring) override {
    // Reset this organism after dividing.
    resources = 0;
    repro_count += 1;
  }

  // Called when *this* organism is born
  // - when offspringready signal is triggered
  // - after mutations
  void OnBirth(this_t & parent) override {
    phenotype.num_ones = genome.CountOnes();
    this->SetDead(false);
    this->SetReproReady(false);
    this->SetNewBorn(true);
  }

  void OnDeath(size_t) override { }

  /// Called when *this* organism is placed
  void OnPlacement(size_t pos) override {
    this->SetWorldID(pos);
    UpdateMerit();
  }

  // NOTE - alternatively, I could just make this a friend of the world class, and pass the world into the process step function
  // todo - make this a virtual base function?
  // NOTE - can't reproduce *while* this organism is executing because I might overwrite => lead to a bad memory access
  // WARNING - Use the world responsibly. If you overwrite the current organism, you can create ugly bad memory access errors.
  template<typename WORLD_T>
  void ProcessStep(WORLD_T& world) {
    // Update resource stockpile
    resources += 1 + phenotype.num_ones; // (accumulate some resources even if all 0s)
    // Enough resources to reproduce?
    SetReproReady(resources >= REPRO_RES_THRESHOLD);
    resources -= ((resources >= REPRO_RES_THRESHOLD) * REPRO_RES_THRESHOLD); // fancy conditional-less way to conditionally subtract threshold if resources exceeds threshold
    // Collecting resources ages you up.
    age += 1;
    // Too old?
    if (age >= MAX_AGE) {
      SetDead(world.GetRandom().P(0.5)); // Arbitrarily, 50% chance to die if too old.
    }
  }

};

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONEMAX_ORGANISM_HPP_INCLUDE