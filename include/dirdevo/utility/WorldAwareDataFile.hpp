#pragma once

#include "emp/data/DataFile.hpp"

namespace dirdevo {

/// A data file that when it updates needs to know which world its recording data for
template<typename WORLD_T>
class WorldAwareDataFile : public emp::DataFile {
public:
  using emp::DataFile::Update;

protected:
  emp::Ptr<WORLD_T> cur_world=nullptr; ///< Non-owning pointer.

public:

  template <typename ...ARGS>
  explicit WorldAwareDataFile(ARGS&& ...arguments)
    : emp::DataFile(std::forward<ARGS>(arguments)...) {;}

  void Update(emp::Ptr<WORLD_T> world) {
    cur_world = world; // Just for this update, set cur_world
    Update();
    cur_world = nullptr;
  }

  WORLD_T& GetCurWorld() {
    emp_assert(cur_world!=nullptr);
    return *cur_world;
  }

};

}