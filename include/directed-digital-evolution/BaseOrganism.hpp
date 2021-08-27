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
  double merit=0.0; /// Used to determine weight in scheduler.
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

  // virtual double UpdateMerit() { emp_assert(false); return 0.0; };
  virtual void OnBeforeRepro() { emp_assert(false, "This function must be implemented by derived organism class."); }
  virtual void OnOffspringReady(DERIVED_T&) { emp_assert(false, "This function must be implemented by derived organism class."); }
  virtual void OnPlacement(size_t) { emp_assert(false, "This function must be implemented by derived organism class."); }
  virtual void OnBirth(DERIVED_T&) {emp_assert(false, "This function must be implemented by derived organism class."); }

};

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_BASE_ORGANISM_HPP_INCLUDE