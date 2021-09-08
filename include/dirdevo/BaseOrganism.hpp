#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_BASE_ORGANISM_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_BASE_ORGANISM_HPP_INCLUDE

#include <cstddef>

namespace dirdevo {

/// BaseOrganism exists to remind me & enforce what I require organism classes to implement...
template<typename DERIVED_T>
class BaseOrganism {
public:

  // using mutator_t = typename DERIVED_T::mutator_t; // DERIVED_T should specify a mutator.
  // DERIVED_T needs to specify a 'mutator_t'

protected:
  double merit=1.0; /// Used to determine weight in scheduler.
  bool new_born=false;
  bool dead=false;
  bool repro_ready=false;
  size_t world_id=0;

public:

  virtual ~BaseOrganism() = default;

  double GetMerit() const { return merit; }
  bool GetNewBorn() const { return new_born; }
  bool GetDead() const { return dead; }
  size_t GetWorldID() const { return world_id; }
  bool GetReproReady() const { return repro_ready; }

  void SetMerit(double m) { merit = m; }
  void SetNewBorn(bool n) { new_born = n; }
  void SetDead(bool d) { dead = d; }
  void SetWorldID(size_t id) { world_id = id; }
  void SetReproReady(bool r) { repro_ready = r; }

  /**
   * Derived organisms need to implement a ProcessStep function:
   *   template<typename WORLD_T>
   *   void ProcessStep(WORLD_T & world) {
   *      ....
   *   }
   */

  /// Called when this organism is about to be injected into the population.
  virtual void OnInjectReady() { emp_assert(false, "Derived task class must implement this function."); }

  /// Called when this organism is about to reproduce (but offspring has not been built yet)
  virtual void OnBeforeRepro() { emp_assert(false, "This function must be implemented by derived organism class."); }

  /// Called when this organism's offspring is ready (after the offspring's OnBirth function is called)
  virtual void OnOffspringReady(DERIVED_T& offspring) { emp_assert(false, "This function must be implemented by derived organism class."); }

  /// Called when *this* organism is placed (just after birth or injection)
  virtual void OnPlacement(size_t position) { emp_assert(false, "This function must be implemented by derived organism class."); }

  /// Called when *this* organism is born
  /// - when world's offspring ready signal is triggered
  /// - after mutations
  virtual void OnBirth(DERIVED_T& parent) { emp_assert(false, "This function must be implemented by derived organism class."); }

  /// Called when *this* organism is being 'killed' by the world (in most cases, this won't do anything)
  /// - Position is the position in the world where the organism is being removed
  virtual void OnDeath(size_t position) { emp_assert(false, "This function must be implemented by the derived organism class."); }

};

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_BASE_ORGANISM_HPP_INCLUDE