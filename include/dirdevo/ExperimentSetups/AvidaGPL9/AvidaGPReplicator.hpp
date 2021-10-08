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

  using input_t = double;
  using output_t = double;
  using input_buffer_t = emp::vector<input_t>;
  using output_buffer_t = emp::vector<output_t>;

protected:

  size_t sites_copied=0;         /// Tracks number of instructions copied by executing copy instructions
  size_t world_id=0;             /// World ID where this hardware unit resides
  bool dividing=false;           /// Did virtual hardware trigger division (self-replication)?
  size_t failed_self_divisions=0;     /// Number of failed division attempts

  size_t num_pathways=0;
  emp::vector<size_t> env_ids;
  emp::vector<size_t> input_pointers;
  emp::vector< input_buffer_t > input_buffers;
  emp::vector< output_buffer_t > output_buffers;

public:

  AvidaGPReplicator(const genome_t & in_genome) :
    AvidaCPU_Base(in_genome),
    num_pathways(1),
    env_ids(1,0),
    input_pointers(1,0),
    input_buffers(1),
    output_buffers(1)
  { ; }

  AvidaGPReplicator(emp::Ptr<const inst_lib_t> inst_lib) :
    AvidaCPU_Base(genome_t(inst_lib)),
    num_pathways(1),
    env_ids(1,0),
    input_pointers(1,0),
    input_buffers(1),
    output_buffers(1)
  { ; }

  AvidaGPReplicator(const inst_lib_t & inst_lib) :
    AvidaCPU_Base(genome_t(&inst_lib)),
    num_pathways(1),
    env_ids(1,0),
    input_pointers(1,0),
    input_buffers(1),
    output_buffers(1)
  { ; }

  AvidaGPReplicator() = default;
  AvidaGPReplicator(const AvidaGPReplicator &) = default;
  AvidaGPReplicator(AvidaGPReplicator &&) = default;

  virtual ~AvidaGPReplicator() { ; }

  void ResetReplicatorHardware(size_t n_pathways) {
    SetNumPathways(n_pathways);
    ResetReplicatorHardware();
  }

  void ResetReplicatorHardware() {
    sites_copied=0;
    dividing=false;
    failed_self_divisions=0;
    for (auto& buffer : input_buffers) {
      buffer.clear();
    }
    for (auto& buffer : output_buffers) {
      buffer.clear();
    }
    std::fill(
      input_pointers.begin(),
      input_pointers.end(),
      0
    );
    ResetHardware();
  }

  void SetNumPathways(size_t n_pathways) {
    emp_assert(n_pathways > 0, "Cannot set number of pathways to 0.", n_pathways);
    num_pathways = n_pathways;
    env_ids.resize(num_pathways, 0);
    input_pointers.resize(num_pathways, 0);
    input_buffers.resize(num_pathways);
    output_buffers.resize(num_pathways);
  }

  size_t GetEnvID(size_t buffer_id=0) const {
    emp_assert(buffer_id < env_ids.size());
    return env_ids[buffer_id];
  }

  void SetEnvID(size_t buffer_id, size_t e_id) {
    emp_assert(buffer_id < env_ids.size());
    env_ids[buffer_id] = e_id;
  }

  // So hardware still works with tasks that don't use multiple buffers
  void SetEnvID(size_t id) {
    SetEnvID(0, id);
  }

  size_t GetWorldID() const { return world_id; }
  void SetWorldID(size_t id) { world_id = id; }

  emp::vector<input_t>& GetInputBuffer(size_t buffer_id=0) {
    emp_assert(buffer_id < input_buffers.size());
    return input_buffers[buffer_id];
  }
  emp::vector<output_t>& GetOutputBuffer(size_t buffer_id=0) {
    emp_assert(buffer_id < output_buffers.size());
    return output_buffers[buffer_id];
  }
  size_t GetInputPointer(size_t buffer_id=0) const {
    emp_assert(buffer_id < input_pointers.size());
    return input_pointers[buffer_id];
  }

  size_t AdvanceInputPointer(size_t buffer_id=0) {
    emp_assert(buffer_id < input_buffers.size());
    emp_assert(buffer_id < input_pointers.size());
    const size_t ret_val=input_pointers[buffer_id];
    input_pointers[buffer_id] = (ret_val+1) % input_buffers[buffer_id].size();
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