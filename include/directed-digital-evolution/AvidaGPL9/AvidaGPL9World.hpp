#pragma once
#ifndef DIRECTED_DEVO_AVIDAGPL9_WORLD_HPP_INCLUDE
#define DIRECTED_DEVO_AVIDAGPL9_WORLD_HPP_INCLUDE

#include <unordered_set>
#include <deque>

#include "../DirectedDevoWorld.hpp"

// TODO - sub namespace for agp-l9?
namespace dirdevo {

template<typename TASK>
class AvidaGPL9World : public DirectedDevoWorld<AvidaGPOrganism, TASK> {

public:

  using this_t = AvidaGPL9World<TASK>;
  using base_t = DirectedDevoWorld<AvidaGPOrganism, TASK>;
  using task_t = TASK;
  using org_t = AvidaGPOrganism;
  using hardware_t = emp::AvidaGP;
  using inst_lib_t = typename hardware_t::inst_lib_t;
  using config_t = typename base_t::config_t;
  using pop_struct_t = typename base_t::pop_struct_t; // TODO - have base class handle structure

  using base_t::POP_STRUCTURE;

protected:

  void SetupInstLib();

  inst_lib_t inst_lib;

public:
  AvidaGPL9World(
    const config_t& cfg,
    emp::Random & rnd,
    const std::string & name="",
    const pop_struct_t & p_struct={}
  ) :
    base_t(cfg, rnd, name, p_struct)
  {
    // Configure the instruction set.
    SetupInstLib();

    std::cout << "AvidaGPL9World constructed." << std::endl;
  }


  const inst_lib_t& GetInstLib() const { return inst_lib; }
};


template<typename TASK>
void AvidaGPL9World<TASK>::SetupInstLib() {
  std::cout << "Setting up instruction library." << std::endl;

  inst_lib = inst_lib_t::DefaultInstLib();

  inst_lib.AddInst(
    "Nop",
    [](hardware_t& hw, const hardware_t::inst_t & inst) {
      return;
    },
    0,
    "No operation"
  );

}





}

#endif // #ifndef DIRECTED_DEVO_AVIDAGPL9_WORLD_HPP_INCLUDE