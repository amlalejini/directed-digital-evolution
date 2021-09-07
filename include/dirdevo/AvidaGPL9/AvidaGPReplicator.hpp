#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_REPLICATOR_HARDWARE_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_REPLICATOR_HARDWARE_HPP_INCLUDE


#include "emp/hardware/Genome.hpp"
#include "emp/hardware/AvidaGP.hpp"

#include "../BaseOrganism.hpp"

// STATUS: In progress

namespace dirdevo {


/// Based on emp::AvidaGP class. Tacks on some hardware components for tracking self-replication.
class AvidaGPReplicator : public emp::AvidaCPU_Base<AvidaGPReplicator> {
public:
  using base_t = AvidaCPU_Base<AvidaGPReplicator>;
  using typename base_t::genome_t;
  using typename base_t::inst_lib_t;

protected:
  size_t sites_copied=0; /// Tracks number of instructions copied by executing copy instructions

public:

  AvidaGPReplicator(const genome_t & in_genome) : AvidaCPU_Base(in_genome) { ; }
  AvidaGPReplicator(emp::Ptr<const inst_lib_t> inst_lib) : AvidaCPU_Base(genome_t(inst_lib)) { ; }
  AvidaGPReplicator(const inst_lib_t & inst_lib) : AvidaCPU_Base(genome_t(&inst_lib)) { ; }

  AvidaGPReplicator() = default;
  AvidaGPReplicator(const AvidaGPReplicator &) = default;
  AvidaGPReplicator(AvidaGPReplicator &&) = default;

  virtual ~AvidaGPReplicator() { ; }

  bool IsDoneCopying() const { return sites_copied >= GetSize(); }
  size_t GetSitesCopied() const { return sites_copied; }
  void SetSitesCopied(size_t copied) { sites_copied = copied; }
  void IncSitesCopied(size_t inc=1) { sites_copied += inc; }

};

}

#endif // #ifndef