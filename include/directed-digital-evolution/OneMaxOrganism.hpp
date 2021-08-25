#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONEMAX_ORGANISM_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_ONEMAX_ORGANISM_HPP_INCLUDE

#include "BaseOrganism.hpp"

namespace dirdevo {

  template<size_t GENOME_SIZE=128>
  class OneMaxOrganism : public BaseOrganism {
  public:
    struct Phenotype;

    using genome_t = emp::BitSet<GENOME_SIZE>;
    using phenotype_t = Phenotype;

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

  public:
    OneMaxOrganism(const genome_t& g) :
      genome(g)
    {
      phenotype.num_ones = genome.CountOnes();
    }

    double GetResources() const { return resources; }
    genome_t & GetGenome() { return genome; }
    phenotype_t & GetPhenotype() { return phenotype; }

    // todo - make a virtual base function?
    double UpdateMerit() {
      merit = 1 + phenotype.num_ones; // +1 ensures that merit can't be zero for living organisms
      return merit;
    }

    // todo - make this a virtual base function?
    void ProcessStep() {
      resources += phenotype.num_ones;
    }


  };

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONEMAX_ORGANISM_HPP_INCLUDE