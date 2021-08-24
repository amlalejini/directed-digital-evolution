#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONEMAX_ORGANISM_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_ONEMAX_ORGANISM_HPP_INCLUDE

#include "BaseOrganism.hpp"

namespace dirdevo {

  template<size_t GENOME_SIZE=128>
  class OneMaxOrganism : BaseOrganism {
  public:
    struct Phenotype;

    using genome_t = emp::BitSet<GENOME_SIZE>;
    using phenotype_t = Phenotype;

    struct Phenotype {
      size_t num_ones=0;
    }

  protected:
    genome_t genome;
    phenotype_t phenotype;

  public:
    OneMaxOrganism(const genome_t& g) :
      genome(g)
    {
      phenotype.num_ones = genome.CountOnes();
    }

    // todo - make a virtual base function?
    double UpdateMerit() {
      merit = phenotype.num_ones;
      return merit;
    }

    static genome_t GenerateAncestralGenome() {
      return genome_t(false);
    }

  };

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONEMAX_ORGANISM_HPP_INCLUDE