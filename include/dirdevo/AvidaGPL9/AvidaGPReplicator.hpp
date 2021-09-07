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

  size_t sites_copied=0;         /// Tracks number of instructions copied by executing copy instructions
  size_t env_id=0;               /// ID of the environment (from the environment bank) for this hardware
  size_t world_id=0;             /// World ID where this hardware unit resides
  bool dividing=false;           /// Did virtual hardware trigger division (self-replication)?
  size_t failed_self_divisions=0;     /// Number of failed division attempts

public:

  AvidaGPReplicator(const genome_t & in_genome) : AvidaCPU_Base(in_genome) { ; }
  AvidaGPReplicator(emp::Ptr<const inst_lib_t> inst_lib) : AvidaCPU_Base(genome_t(inst_lib)) { ; }
  AvidaGPReplicator(const inst_lib_t & inst_lib) : AvidaCPU_Base(genome_t(&inst_lib)) { ; }

  AvidaGPReplicator() = default;
  AvidaGPReplicator(const AvidaGPReplicator &) = default;
  AvidaGPReplicator(AvidaGPReplicator &&) = default;

  virtual ~AvidaGPReplicator() { ; }

  void ResetReplicatorHardware() {
    sites_copied=0;
    dividing=false;
    failed_self_divisions=0;
    ResetHardware();
  }

  size_t GetEnvID() const { return env_id; }
  void SetEnvID(size_t id) { env_id = id; }

  size_t GetWorldID() const { return world_id; }
  void SetWorldID(size_t id) { world_id = id; }

  bool IsDividing() const { return dividing; }
  void SetDividing(bool d) { dividing = d; }

  size_t GetNumFailedSelfDivisions() const { return failed_self_divisions; }
  void IncFailedSelfDivisions(size_t inc=1) { failed_self_divisions += 1; }

  bool IsDoneCopying() const { return sites_copied >= GetSize(); }
  size_t GetSitesCopied() const { return sites_copied; }
  void SetSitesCopied(size_t copied) { sites_copied = copied; }
  void IncSitesCopied(size_t inc=1) { sites_copied += inc; }

};

}

#endif // #ifndef