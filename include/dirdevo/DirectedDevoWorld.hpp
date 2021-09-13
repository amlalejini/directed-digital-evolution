#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_WORLD_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_WORLD_HPP_INCLUDE

#include <unordered_set>
#include <deque>

#include "emp/Evolve/World.hpp"
#include "emp/datastructs/IndexMap.hpp"

#include "utility/ProbabilisticScheduler.hpp"
#include "DirectedDevoConfig.hpp"
#include "utility/ConfigSnapshotEntry.hpp"

namespace dirdevo {

// TODO - add world peripheral??? => Can hold instruction sets?
//        OR, assume that directeddevoworld is not the end point? that is, you need to derive from it
// TODO - clean up configuration (let the world configure more of itself (move out of the experiment..)!)
template <typename ORG, typename TASK>
class DirectedDevoWorld : public emp::World<ORG> {
public:
  friend TASK; // Let the task see my insides. TASK should be sure to be a responsible friend...

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
  using this_t = DirectedDevoWorld<ORG, TASK>;
  using task_t = TASK;
  using scheduler_t = ProbabilisticScheduler;
  using pop_struct_t = PopStructureDesc;
  using org_t = ORG;
  using config_t = DirectedDevoConfig;

  static bool IsValidPopStructure(const std::string & mode);
  static POP_STRUCTURE PopStructureStrToMode(const std::string & mode);

  static void AttachWorldUpdateDataFileFunctions(
    emp::DataFile& summary_file,
    const std::function<emp::Ptr<this_t>(void)>& get_world
  ) {
    summary_file.AddFun<size_t>(
      [get_world]() {
        return get_world()->GetUpdate();
      },
      "world_update"
    );
    summary_file.AddFun<size_t>(
      [get_world]() {
        return get_world()->GetWorldID();
      },
      "world_id"
    );
    summary_file.AddFun<size_t>(
      [get_world]() {
        return get_world()->GetNumOrgs();
      },
      "num_orgs"
    );
    task_t::AttachWorldUpdateDataFileFunctions(summary_file, get_world);
  }

protected:

  using base_t::pop;
  using base_t::name;
  using base_t::control;
  using base_t::on_death_sig;

  const config_t& config; ///< Reference to the experiment's configuration.
  size_t max_pop_size=0;              /// Maximum population size (depends on population structure and configuration)
  size_t avg_org_steps_per_update=1;  /// Determines the number of execution steps we dish out each update (population size * this).
  bool extinct=false;                 /// flag for whether of not the population is extinct
  scheduler_t scheduler;              /// Used to schedule organism execution based on their merit.
  task_t task;                        /// Used to track task performance
  std::function<double(void)> aggregate_performance_fun;
  pop_struct_t pop_struct;
  size_t world_id=0;

  void SetPopStructure(const pop_struct_t & pop_struct); // TODO - clean this up more!

public:

  // using base_t::base_t;
  DirectedDevoWorld(
    const config_t& cfg,
    emp::Random & rnd,
    const std::string & name="",
    size_t id=0
  ) :
    base_t(rnd, name),
    config(cfg),
    scheduler(rnd),
    task(*this),
    pop_struct(
      this_t::PopStructureStrToMode(cfg.LOCAL_POP_STRUCTURE()),
      cfg.LOCAL_GRID_WIDTH(),
      cfg.LOCAL_GRID_HEIGHT(),
      cfg.LOCAL_GRID_DEPTH()
    ),
    world_id(id)
  {

    /// TODO - document the order of signal calls in the world!
    /// TODO - is there a way to strip out unused functions?

    // Wire up event handles to world signals.
    // - Update scheduler weights on organism placement, death, and swap.
    // - Tell task about placement, death, etc
    // - Tell organism about placement, death, etc

    // NOTE - reminder that on placement signal will still trigger for injected organisms!
    this->OnInjectReady(
      [this](org_t& org) {
        org.OnInjectReady();
        task.OnOrgInjectReady(org);
      }
    );

    this->OnPlacement(
      [this](size_t pos) {
        auto& org = this->GetOrg(pos);
        org.OnPlacement(pos);                        // Tell the organism about its placement.
        task.OnOrgPlacement(org, pos);               // Tell the task about organism placement.
        scheduler.AdjustWeight(pos, org.GetMerit()); // Update scheduler weights last.
        extinct=false; // World can't be extinct anymore
      }
    );

    auto org_death_key = this->OnOrgDeath(
      [this](size_t pos) {
        auto& org = this->GetOrg(pos);
        org.OnDeath(pos);
        task.OnOrgDeath(org, pos);
        scheduler.AdjustWeight(pos, 0); // Update scheduler weights last.
      }
    );

    // We need to remove the on death signal to prevent it from being triggered when the world clears the population (calling remove org)
    this->OnWorldDestruct(
      [this,org_death_key]() {
        on_death_sig.Remove(org_death_key);
      }
    );

    this->OnSwapOrgs(
      [this](emp::WorldPosition p1, emp::WorldPosition p2) {
        auto& org1 = this->GetOrg(p1.GetIndex());
        auto& org2 = this->GetOrg(p2.GetIndex());
        // Organisms have already been swapped, so p1 org needs index to reflect p1; same with p2 org.
        org1.SetWorldID(p1.GetIndex());
        org2.SetWorldID(p2.GetIndex());

        task.AfterOrgSwap(org1, org2);

        // Update scheduler weights last
        const auto& weight_map = scheduler.GetWeightMap();
        const double p1_weight = weight_map.GetWeight(p1.GetIndex());
        const double p2_weight = weight_map.GetWeight(p2.GetIndex());
        scheduler.AdjustWeight(p1.GetIndex(), p2_weight);
        scheduler.AdjustWeight(p2.GetIndex(), p1_weight);
      }
    );

    // Organism about to reproduce. Before building offspring.
    this->OnBeforeRepro(
      [this](size_t parent_pos) {
        auto& parent = this->GetOrg(parent_pos);
        parent.OnBeforeRepro();     // Tell parent that it's about to reproduce
        task.OnBeforeOrgRepro(parent); // Tell task about reproduction
      }
    );

    // Offspring constructed, but has not been placed.
    // Last time to safely access parent.
    this->OnOffspringReady(
      [this](org_t& offspring, size_t parent_pos) {
        this->DoMutationsOrg(offspring); // Do mutations on offspring ready, but before parent sees offspring.
        auto& parent = this->GetOrg(parent_pos);
        offspring.OnBirth(parent);                // Tell offspring about it's birthday!
        parent.OnOffspringReady(offspring);       // Tell parent that it's offspring is ready
        task.OnOffspringReady(offspring, parent); // Tell task that this offspring was born from this parent.
      }
    );

    // this->OnUpdate( // Removed this to guarantee it is called before
    //   [this](size_t u) {
    //     task.OnWorldUpdate(u);
    //   }
    // );

    // Configure population structure.
    SetPopStructure(pop_struct);

    task.OnWorldSetup(); // Tell the task that the world has been configured.
    aggregate_performance_fun = task.GetAggregatePerformanceFun(); // TODO - test that this wiring works as expected!

  }

  const std::string& GetName() const { return name; }
  size_t GetWorldID() const { return world_id; }

  bool IsExtinct() const { return extinct; }

  /// Configure the average number of steps distributed to each organism per world update
  void SetAvgOrgStepsPerUpdate(size_t avg_steps);

  /// Force a re-sync of scheduler weights with organism merits
  void SyncSchedulerWeights();

  /// Run world one step (update) forward
  void RunStep();

  /// Run world forward for given number of updates
  // void Run(size_t updates);

  /// Evaluate the world (make sure task performance is current)
  void Evaluate();

  void DirectedDevoReset();

  double GetAggregateTaskPerformance() { emp_assert(task.IsEvalFresh()); return aggregate_performance_fun(); }
  double GetSubTaskPerformance(size_t i);

  emp::vector<ConfigSnapshotEntry> GetConfigSnapshotEntries() {
    emp::vector<ConfigSnapshotEntry> entries(task.GetConfigSnapshotEntries()); // Grab all of the task-specific entries
    // Add world entries
    entries.emplace_back(
      "world_size",
      emp::to_string(this->GetSize()),
      "world__" + GetName()
    );
    return entries;
  }

  size_t GetNumSubTasks();

  task_t& GetTask() { return task; }
  const task_t& GetTask() const { return task; }

  const config_t& GetConfig() const { return config; }

};

template<typename ORG, typename TASK>
void DirectedDevoWorld<ORG,TASK>::SetPopStructure(
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

template<typename ORG, typename TASK>
void DirectedDevoWorld<ORG,TASK>::SetAvgOrgStepsPerUpdate(size_t avg_steps) {
  avg_org_steps_per_update=avg_steps;
  scheduler.Reset(max_pop_size, avg_org_steps_per_update*max_pop_size);
  SyncSchedulerWeights();
}

template<typename ORG, typename TASK>
void DirectedDevoWorld<ORG,TASK>::SyncSchedulerWeights() {
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

// template<typename ORG, typename TASK>
// void DirectedDevoWorld<ORG,TASK>::Run(size_t updates) {
//   for (size_t u = 0; u < updates; u++) {
//     RunStep();
//   }
// }

template<typename ORG, typename TASK>
void DirectedDevoWorld<ORG,TASK>::RunStep() {
  // Tell task that we're about to run an update
  task.OnBeforeWorldUpdate(this->GetUpdate());

  // Check assumptions about the state of the world.
  const size_t num_orgs = this->GetNumOrgs();
  extinct = !(bool)num_orgs;
  if (extinct) {
    return;  // If there are no organisms alive, do nothing (world has gone extinct).
  }
  // emp_assert(scheduler.GetWeightMap().GetWeight() > 0, "Scheduler requires total weight > 0.");

  /////////////////////////////////////////////////////////////////
  // std::cout << "-------------- RUN STEP (" << this->GetUpdate() << ") --------------" << std::endl;
  /////////////////////////////////////////////////////////////////

  // --- Beyond this point: assume that the scheduler weights are current and up-to-date ---
  // Compute how many organism steps we can dish out for this world update!
  const size_t org_step_budget = num_orgs*avg_org_steps_per_update;
  for (size_t step = 0; step < org_step_budget; ++step) {
    // Schedule someone to take a step.
    const size_t org_id = scheduler.GetRandom(); // This should reweight the scheduler automatically.
    auto & org = this->GetOrg(org_id);
    // Step organism forward
    task.BeforeOrgProcessStep(org);
    org.ProcessStep(*this);
    task.AfterOrgProcessStep(org);
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

  // if (this->GetUpdate() == 4095 ) {
  //   /////////////////////////////////////////////////////////////////
  //   std::cout << "  Population:";
  //   for (size_t i = 0; i < pop.size(); ++i) {
  //     if (this->IsOccupied(i)) {
  //       std::cout << " {"
  //         << "id:"<<i<<","
  //         << "merit:"<<pop[i]->GetMerit();
  //         // << "pheno:"<<pop[i]->GetPhenotype().num_ones << ",";
  //         // << "ones:"<< pop[i]->GetGenome().CountOnes();
  //     } else {
  //       std::cout << " {id:"<<i<<","<<"dead";
  //     }
  //     std::cout << ",weight:"<<scheduler.GetWeightMap().GetWeight(i)<<"}";
  //   }
  //   std::cout << std::endl;
  //   // std::cout << "  Resource levels (after update): ";
  //   // for (size_t i = 0; i < this->GetSize(); ++i) {
  //   //   if (this->IsOccupied(i)) std::cout << " {id-"<<i<<" " << this->GetOrg(i).GetResources() << "}";
  //   // }
  //   // std::cout << std::endl;
  //   /////////////////////////////////////////////////////////////////
  // }

  // TODO - any data recording, etc here

  // Update the world
  task.OnWorldUpdate(this->GetUpdate()); // Guarantee that this is called before externally-attached on update functions
  this->Update();
}

template<typename ORG, typename TASK>
void DirectedDevoWorld<ORG,TASK>::Evaluate() {
  task.Evaluate();
}

template<typename ORG, typename TASK>
void DirectedDevoWorld<ORG,TASK>::DirectedDevoReset() {
  task.OnWorldReset();          // Tell task that the world is being reset.
  base_t::Reset();              // Call base reset function.
  SetPopStructure(pop_struct);  // Reset the population structure.
}

template<typename ORG, typename TASK>
bool DirectedDevoWorld<ORG,TASK>::IsValidPopStructure(const std::string & mode) {
  return emp::Has({"mixed", "grid", "grid3d"}, mode);
}

template<typename ORG, typename TASK>
typename DirectedDevoWorld<ORG,TASK>::POP_STRUCTURE DirectedDevoWorld<ORG,TASK>::PopStructureStrToMode(const std::string & mode) {
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