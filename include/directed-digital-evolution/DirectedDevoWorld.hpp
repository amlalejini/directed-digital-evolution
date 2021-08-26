#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_WORLD_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_WORLD_HPP_INCLUDE

#include <unordered_set>
#include <deque>

#include "emp/Evolve/World.hpp"
#include "emp/datastructs/IndexMap.hpp"

#include "ProbabilisticScheduler.hpp"

namespace dirdevo {

template <typename ORG>
class DirectedDevoWorld : public emp::World<ORG> {
public:
  // todo - wrap this in a struct?
  enum class POP_STRUCTURE { MIXED, GRID, GRID3D };
  struct PopStructureDesc {

    POP_STRUCTURE mode;
    size_t width;
    size_t height;
    size_t depth;

    PopStructureDesc(
      POP_STRUCTURE m=POP_STRUCTURE::MIXED,
      size_t w=2,
      size_t h=2,
      size_t d=1
    ) :
      mode(m),
      width(w),
      height(h),
      depth(d)
    { ; }

  };

  using base_t = emp::World<ORG>;
  using this_t = DirectedDevoWorld<ORG>;
  using scheduler_t = ProbabilisticScheduler;
  using pop_struct_t = PopStructureDesc;
  using org_t = ORG;

  static bool IsValidPopStructure(const std::string & mode);
  static POP_STRUCTURE PopStructureStrToMode(const std::string & mode);

protected:

  using base_t::pop;
  using base_t::name;
  using base_t::control;

  size_t max_pop_size=0;
  size_t avg_org_steps_per_update=1;
  scheduler_t scheduler;     /// Used to schedule organism execution based on their merit.

  // emp::Signal<void(size_t)> reproduce_sig; /// Organisms trigger this signal when ready to reproduce.
  // emp::Signal<void(size_t)> kill_sig;       /// Organisms trigger this signal to mark themselves as dead.

  // std::deque<size_t> repro_queue;   /// Queue of organisms marked for reproduction
  // std::deque<size_t> death_queue;   /// Queue of organisms marked for death

  void SetPopStructure(const pop_struct_t & pop_struct);

public:

  // using base_t::base_t;
  DirectedDevoWorld(
    emp::Random & rnd,
    const std::string & name="",
    const pop_struct_t & pop_struct={}
  ) :
    base_t(rnd, name),
    scheduler(rnd)
  {

    // Wire up scheduler to the world.
    // - Update scheduler weights on organism placement, death, and swap.
    this->OnPlacement(
      [this](size_t pos) {
        auto& org = this->GetOrg(pos);
        org.SetWorldID(pos);
        org.SetDead(false);
        org.SetReproReady(false);
        org.SetNewBorn(true); // TODO - do we really want to set this here?
        scheduler.AdjustWeight(pos, org.UpdateMerit()); // Being alive gets you at least one merit!
      }
    );

    this->OnOrgDeath(
      [this](size_t pos) {
        // auto & org = this->GetOrg(pos);
        scheduler.AdjustWeight(pos, 0);
      }
    );

    this->OnSwapOrgs(
      [this](emp::WorldPosition p1, emp::WorldPosition p2) {
        const auto& weight_map = scheduler.GetWeightMap();
        const double p1_weight = weight_map.GetWeight(p1.GetIndex());
        const double p2_weight = weight_map.GetWeight(p2.GetIndex());
        scheduler.AdjustWeight(p1.GetIndex(), p2_weight);
        scheduler.AdjustWeight(p2.GetIndex(), p1_weight);

        auto& org1 = this->GetOrg(p1.GetIndex());
        auto& org2 = this->GetOrg(p2.GetIndex());
        // Organisms have already been swapped, so p1 org needs index to reflect p1; same with p2 org.
        org1.SetWorldID(p1.GetIndex());
        org2.SetWorldID(p2.GetIndex());
      }
    );

    // Organism about to reproduce. Before building offspring.
    this->OnBeforeRepro(
      [this](size_t parent_pos) {
        auto& parent = this->GetOrg(parent_pos);
        parent.OnBeforeRepro();
      }
    );

    // Offspring constructed, but has not been placed.
    // Last time to safely access parent.
    this->OnOffspringReady(
      [this](org_t& offspring, size_t parent_pos) {
        auto& parent = this->GetOrg(parent_pos);
        parent.OnOffspringReady(offspring);
      }
    );

    // TODO - setup mutation

    // Configure population structure.
    SetPopStructure(pop_struct);
  }

  const std::string & GetName() const { return name; }

  /// Configure the average number of steps distributed to each organism per world update
  void SetAvgOrgStepsPerUpdate(size_t avg_steps);

  /// Force a re-sync of scheduler weights with organism merits
  void SyncSchedulerWeights();

  // void TriggerReproduction(size_t ) { reproduce_sig.};

  // void TriggerKill();

  /// Run world one step (update) forward
  void RunStep();

  /// Run world forward for given number of updates
  void Run(size_t updates); // todo
};

template<typename ORG>
void DirectedDevoWorld<ORG>::SetPopStructure(
  const PopStructureDesc& pop_struct
)
{
  switch(pop_struct.mode) {
    case POP_STRUCTURE::MIXED: {
      max_pop_size=pop_struct.width*pop_struct.height;
      this->SetPopStruct_Mixed(false);
      this->Resize(max_pop_size); // Mixed structure doesn't resize the world, do so here.
      break;
    }
    case POP_STRUCTURE::GRID: {
      max_pop_size=pop_struct.width*pop_struct.height;
      this->SetPopStruct_Grid(pop_struct.width, pop_struct.height, false);
      break;
    }
    case POP_STRUCTURE::GRID3D: {
      max_pop_size=pop_struct.width*pop_struct.height*pop_struct.depth;
      this->SetPopStruct_3DGrid(pop_struct.width, pop_struct.height, pop_struct.depth, false);
      break;
    }
  }
  // Setup the scheduler
  scheduler.Reset(max_pop_size, avg_org_steps_per_update*max_pop_size);
}

template<typename ORG>
void DirectedDevoWorld<ORG>::SetAvgOrgStepsPerUpdate(size_t avg_steps) {
  avg_org_steps_per_update=avg_steps;
  scheduler.Reset(max_pop_size, avg_org_steps_per_update*max_pop_size);
  // TODO update weights in scheduler!
}

template<typename ORG>
void DirectedDevoWorld<ORG>::SyncSchedulerWeights() {
  scheduler.DeferWeightRefresh(); // Bulk adjustments, defer refresh until next index
  for (size_t i = 0; i < pop.size(); ++i) {
    if (this->IsOccupied(i)) {
      const double merit = pop[i]->GetMerit();
      scheduler.AdjustWeight(i, merit);
    } else {
      scheduler.AdjustWeight(i, 0);
    }
  }
}

template<typename ORG>
void DirectedDevoWorld<ORG>::RunStep() {
  // Check assumptions about the state of the world.
  const size_t num_orgs = this->GetNumOrgs();
  if (!num_orgs) return;  // If there are no organisms alive, do nothing.
  // emp_assert(scheduler.GetWeightMap().GetWeight() > 0, "Scheduler requires total weight > 0.");

  /////////////////////////////////////////////////////////////////
  std::cout << "Population:";
  for (size_t i = 0; i < pop.size(); ++i) {
    if (this->IsOccupied(i)) {
      std::cout << " {id:"<<i<<","<<"merit:"<<pop[i]->GetMerit();
    } else {
      std::cout << " {id:"<<i<<","<<"dead";
    }
    std::cout << ",weight:"<<scheduler.GetWeightMap().GetWeight(i)<<"}";
  }
  std::cout << std::endl;
  /////////////////////////////////////////////////////////////////

  // --- Beyond this point: assume that the scheduler weights are current and up-to-date ---
  // Compute how many organism steps we can dish out for this world update!
  const size_t org_step_budget = this->GetNumOrgs()*avg_org_steps_per_update;
  for (size_t step = 0; step < org_step_budget; ++step) {
    // Schedule someone to take a step.
    const size_t org_id = scheduler.GetRandom(); // This should reweight the scheduler automatically.
    auto & org = this->GetOrg(org_id);
    // Step organism forward
    org.ProcessStep(*this);
    // Should organism reproduce?
    if (org.GetReproReady()) {
      auto offspring_pos = this->DoBirth(org.GetGenome(), org_id, 1);
      // If this organism's offspring stomped all over it, we should jump over to the next iteration of the loop
      if (offspring_pos.GetIndex() == org_id) continue;
    }
    // should this organism die?
    if (org.GetDead()) {
      this->DoDeath({org_id});
    }
  }

  /////////////////////////////////////////////////////////////////
  std::cout << "Resource levels: ";
  for (size_t i = 0; i < this->GetSize(); ++i) {
    if (this->IsOccupied(i)) std::cout << " {id-"<<i<<" " << this->GetOrg(i).GetResources() << "}";
  }
  std::cout << std::endl;
  /////////////////////////////////////////////////////////////////

}

template<typename ORG>
bool DirectedDevoWorld<ORG>::IsValidPopStructure(const std::string & mode) {
  return emp::Has({"mixed", "grid", "grid3d"}, mode);
}

template<typename ORG>
typename DirectedDevoWorld<ORG>::POP_STRUCTURE DirectedDevoWorld<ORG>::PopStructureStrToMode(const std::string & mode) {
  emp_assert(IsValidPopStructure(mode), "Invalid population structure string.");
  static std::unordered_map<std::string, POP_STRUCTURE> pop_struct_str_to_mode = {
    {"mixed", POP_STRUCTURE::MIXED},
    {"grid", POP_STRUCTURE::GRID},
    {"grid3d", POP_STRUCTURE::GRID3D}
  };
  return pop_struct_str_to_mode[mode];
}

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_WORLD_HPP_INCLUDE