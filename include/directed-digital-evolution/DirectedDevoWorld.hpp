#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_WORLD_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_WORLD_HPP_INCLUDE

#include "emp/Evolve/World.hpp"

namespace dirdevo {

template <typename ORG>
class DirectedDevoWorld : public emp::World<ORG> {
public:
  // todo - wrap this in a struct?
  enum class POP_STRUCTURE { MIXED, GRID, GRID3D };

  using base_t = emp::World<ORG>;
  using this_t = DirectedDevoWorld<ORG>;


protected:

  size_t max_pop_size=0;

public:

  // using base_t::base_t;
  DirectedDevoWorld(emp::Random & rnd, const std::string & name="") : base_t(rnd, name) {;}

  // TODO - anything else necessary to setup population structure...
  void SetPopStructure(
    POP_STRUCTURE mode,
    size_t width,
    size_t height,
    size_t depth=1
  ) {
    switch(mode) {
      case POP_STRUCTURE::MIXED: {
        this->SetPopStruct_Mixed(false);
        max_pop_size=width*height;
        break;
      }
      case POP_STRUCTURE::GRID: {
        this->SetPopStruct_Grid(width, height, false);
        max_pop_size=width*height;
        break;
      }
      case POP_STRUCTURE::GRID3D: {
        this->SetPopStruct_3DGrid(width, height, depth, false);
        max_pop_size=width*height*depth;
        break;
      }
    }
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