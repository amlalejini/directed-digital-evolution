#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_REPLICATOR_HARDWARE_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_AVIDAGP_REPLICATOR_HARDWARE_HPP_INCLUDE

#include <cstddef>

#include "emp/hardware/Genome.hpp"
#include "emp/hardware/AvidaGP.hpp"
#include "emp/base/vector.hpp"

#include "../../BaseOrganism.hpp"

// STATUS: In progress

namespace dirdevo {


/// Based on emp::AvidaGP class. Tacks on some hardware components for tracking self-replication.
class AvidaGPReplicator : public emp::AvidaCPU_Base<AvidaGPReplicator> {
public:
  using base_t = AvidaCPU_Base<AvidaGPReplicator>;
  using typename base_t::genome_t;
  using typename base_t::inst_lib_t;

  using input_t = uint32_t;
  using output_t = uint32_t;

protected:

  size_t sites_copied=0;         /// Tracks number of instructions copied by executing copy instructions
  size_t env_id=0;               /// ID of the environment (from the environment bank) for this hardware
  size_t world_id=0;             /// World ID where this hardware unit resides
  bool dividing=false;           /// Did virtual hardware trigger division (self-replication)?
  size_t failed_self_divisions=0;     /// Number of failed division attempts

  emp::vector<input_t> input_buffer;
  emp::vector<output_t> output_buffer;
  size_t input_pointer=0;

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
    input_buffer.clear();
    output_buffer.clear();
    input_pointer=0;
    ResetHardware();
  }

  size_t GetEnvID() const { return env_id; }
  void SetEnvID(size_t id) { env_id = id; }

  size_t GetWorldID() const { return world_id; }
  void SetWorldID(size_t id) { world_id = id; }

  emp::vector<input_t>& GetInputBuffer() { return input_buffer; }
  emp::vector<output_t>& GetOutputBuffer() { return output_buffer; }
  size_t GetInputPointer() const { return input_pointer; }

  size_t AdvanceInputPointer() {
    emp_assert(input_buffer.size());
    const size_t ret_val=input_pointer;
    input_pointer = (input_pointer+1) % input_buffer.size();
    return ret_val;
  }

  bool IsDividing() const { return dividing; }
  void SetDividing(bool d) { dividing = d; }

  size_t GetNumFailedSelfDivisions() const { return failed_self_divisions; }
  void IncFailedSelfDivisions(size_t inc=1) { failed_self_divisions += inc; }

  bool IsDoneCopying() const { return sites_copied >= GetSize(); }
  size_t GetSitesCopied() const { return sites_copied; }
  void SetSitesCopied(size_t copied) { sites_copied = copied; }
  void IncSitesCopied(size_t inc=1) { sites_copied += inc; }

};

}

#endif // #ifndef