#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_BASE_ORGANISM_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_BASE_ORGANISM_HPP_INCLUDE

#include <cstddef>

namespace dirdevo {

  class BaseOrganism {
  public:
  protected:
    double merit=0.0; /// Used to determine weight in scheduler.
    bool new_born=false;
    bool dead=false;
    size_t world_id=0;

  public:

    double GetMerit() const { return merit; }
    bool GetNewBorn() const { return new_born; }
    bool GetDead() const { return dead; }
    size_t GetWorldID() const { return world_id; }

    void SetMerit(double m) { merit = m; }
    void SetNewBorn(bool n) { new_born = n; }
    void SetDead(bool d) { dead = d; }
    void SetWorldID(size_t id) { world_id = id; }

  };

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_BASE_ORGANISM_HPP_INCLUDE