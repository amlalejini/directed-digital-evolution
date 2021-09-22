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
#include "utility/WorldAwareDataFile.hpp"

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

  // Helpful type aliases
  using base_t = emp::World<ORG>;
  using this_t = DirectedDevoWorld<ORG, TASK>;
  using task_t = TASK;
  using scheduler_t = ProbabilisticScheduler;
  using pop_struct_t = PopStructureDesc;
  using org_t = ORG;
  using genome_t = typename base_t::genome_t;
  using config_t = DirectedDevoConfig;
  using systematics_t = emp::Systematics<org_t, genome_t>; // TODO - work out how to add on extra taxon-associated data tracking if necessary!
  using taxon_t = typename systematics_t::taxon_t;

  // Public functions in base type that we want to use w/out this reference
  using base_t::GetUpdate;
  using base_t::GetOrg;
  using base_t::GetSize;
  using base_t::GetNumOrgs;

  static bool IsValidPopStructure(const std::string & mode);
  static POP_STRUCTURE PopStructureStrToMode(const std::string & mode);

  static void AttachWorldUpdateDataFileFunctions(
    WorldAwareDataFile<this_t>& summary_file
  ) {
    summary_file.template AddFun<size_t>(
      [&summary_file]() {
        return summary_file.GetCurWorld().GetUpdate();
      },
      "world_update"
    );
    summary_file.template AddFun<size_t>(
      [&summary_file]() {
        return summary_file.GetCurWorld().GetWorldID();
      },
      "world_id"
    );
    summary_file.template AddFun<size_t>(
      [&summary_file]() {
        return summary_file.GetCurWorld().GetNumOrgs();
      },
      "num_orgs"
    );
    task_t::AttachWorldUpdateDataFileFunctions(summary_file);
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
  size_t cur_epoch=0;
  bool track_systematics=false;

  /// Wraps the shared
  // TODO - setup ability to strip out systematics tracking (because it can be a performance hit)
  struct SharedSystematicsWrapper {
    emp::Ptr<systematics_t> sys_ptr; ///< NON-OWNING. Pointer to the systematics manager shared by each world in an experiment.
    size_t offset=0;                 ///< world_id*GetSize()
    size_t time_offset=0;

    void SetNextParent(size_t pos) {
      emp_assert(sys_ptr);
      sys_ptr->SetNextParent(pos + offset);
    }

    // TODO - fix time!
    void AddOrg(org_t& org, size_t pos, size_t update) {
      emp_assert(sys_ptr);
      // From the systematics manager's perspective, all worlds are part of pop_0 (for their WorldPosition args)
      sys_ptr->AddOrg(org, {offset+pos, 0}, (int)(update+time_offset));
    }

    void RemoveOrgAfterRepro(size_t pos, size_t update) {
      emp_assert(sys_ptr);
      sys_ptr->RemoveOrgAfterRepro({offset+pos, 0}, (int)(update+time_offset));
      // TODO
    }

    void Update() {
      emp_assert(sys_ptr);
      sys_ptr->Update();
    }

    /// Is the shared systematics manager active?
    bool IsActive() const { return sys_ptr != nullptr; }

  } shared_systematics_wrapper;

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
        if (track_systematics) {
          shared_systematics_wrapper.AddOrg(org, pos, GetUpdate());
        }
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
        if (track_systematics) shared_systematics_wrapper.RemoveOrgAfterRepro(pos, GetUpdate());
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
        if (track_systematics) shared_systematics_wrapper.SetNextParent(parent_pos); // TODO - only call this if shared systematics setup
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

  SharedSystematicsWrapper& GetSharedSystematics() { return shared_systematics_wrapper; }

  bool IsExtinct() const { return extinct; }

  /// Configure the average number of steps distributed to each organism per world update
  void SetAvgOrgStepsPerUpdate(size_t avg_steps);

  /// Configure shared systematics
  void SetSharedSystematics(emp::Ptr<systematics_t> sys, size_t max_world_size) {
    // if (shared_systematics_wrapper.sys_ptr) return; // Currently, this might do wonky things if called twice?
    shared_systematics_wrapper.sys_ptr = sys;
    shared_systematics_wrapper.offset = world_id * max_world_size;
    track_systematics = true;
    std::cout << "Systematics offset ("<<world_id<<"): " << shared_systematics_wrapper.offset << std::endl;
  }

  void SetEpoch(size_t epoch) {
    cur_epoch = epoch;
    shared_systematics_wrapper.time_offset = epoch * config.UPDATES_PER_EPOCH();
  }

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
  double GetSubTaskPerformance(size_t i) {
    emp_assert(task.IsEvalFresh());
    const auto& fun_set = task.GetPerformanceFunSet();
    emp_assert(i < fun_set.size(), i, fun_set.size());
    return fun_set[i]();
  };
  size_t GetNumSubTasks() {
    const auto& fun_set = task.GetPerformanceFunSet();
    return fun_set.size();
  }

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

template<typename ORG, typename TASK>
void DirectedDevoWorld<ORG,TASK>::RunStep() {
  // Tell task that we're about to run an update
  task.OnBeforeWorldUpdate(GetUpdate());

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

  // TODO - any data recording, etc here

  // Update the world
  task.OnWorldUpdate(GetUpdate()); // Guarantee that this is called before externally-attached on update functions
  if (track_systematics) shared_systematics_wrapper.Update();
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