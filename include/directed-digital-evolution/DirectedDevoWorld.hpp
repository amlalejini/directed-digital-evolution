#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_WORLD_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_WORLD_HPP_INCLUDE

#include "emp/Evolve/World.hpp"
#include "emp/datastructs/IndexMap.hpp"

#include "ProbabilisticScheduler.hpp"

namespace dirdevo {

template <typename ORG>
class DirectedDevoWorld : public emp::World<ORG> {
public:
  // todo - wrap this in a struct?
  enum class POP_STRUCTURE { MIXED, GRID, GRID3D };

  using base_t = emp::World<ORG>;
  using this_t = DirectedDevoWorld<ORG>;
  using scheduler_t = ProbabilisticScheduler;

protected:

  size_t max_pop_size=0;
  size_t avg_org_steps_per_update=1;
  scheduler_t scheduler;     /// Used to schedule organism execution based on their merit.


public:

  // using base_t::base_t;
  DirectedDevoWorld(
    emp::Random & rnd,
    const std::string & name=""
  ) :
    base_t(rnd, name),
    scheduler(rnd)
  {
    // Wire up scheduler to the world.
    // - on_placement_sig
    // - on_death_sig
    // - on_swap_sig
    OnPlacement(
      [this](size_t pos) {
        auto & org = GetOrg(pos);
        scheduler.AdjustWeight(pos, org.UpdateMerit());
      }
    );

    OnDeath(
      [this](size_t pos) {
        auto & org = GetOrg(pos);
        scheduler.AdjustWeight(pos, 0);
      }
    );

    // TODO - OnSwap

  }

  // TODO - anything else necessary to setup population structure...
  // TODO - move this to the constructor? shouldn't use this world without doing this...
  void SetPopStructure(
    POP_STRUCTURE mode,
    size_t width,
    size_t height,
    size_t depth=1
  ) {
    switch(mode) {
      case POP_STRUCTURE::MIXED: {
        max_pop_size=width*height;
        this->SetPopStruct_Mixed(false);
        this->Resize(max_pop_size); // Mixed structure doesn't resize the world, do so here.
        break;
      }
      case POP_STRUCTURE::GRID: {
        max_pop_size=width*height;
        this->SetPopStruct_Grid(width, height, false);
        break;
      }
      case POP_STRUCTURE::GRID3D: {
        max_pop_size=width*height*depth;
        this->SetPopStruct_3DGrid(width, height, depth, false);
        break;
      }
    }
    // Setup the scheduler
    scheduler.Reset(max_pop_size, avg_org_steps_per_update*max_pop_size);
  }

  void SetAvgOrgStepsPerUpdate(size_t avg_steps) {
    avg_org_steps_per_update=avg_steps;
    scheduler.Reset(max_pop_size, avg_org_steps_per_update*max_pop_size);
    // TODO update weights in scheduler!
  }

  static bool IsValidPopStructure(const std::string & mode) {
    return emp::Has({"mixed", "grid", "grid3d"}, mode);
  }

  static POP_STRUCTURE PopStructureStrToMode(const std::string & mode) {
    emp_assert(IsValidPopStructure(mode), "Invalid population structure string.");
    static std::unordered_map<std::string, POP_STRUCTURE> pop_struct_str_to_mode = {
      {"mixed", POP_STRUCTURE::MIXED},
      {"grid", POP_STRUCTURE::GRID},
      {"grid3d", POP_STRUCTURE::GRID3D}
    };
    return pop_struct_str_to_mode[mode];
  }
};

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_WORLD_HPP_INCLUDE