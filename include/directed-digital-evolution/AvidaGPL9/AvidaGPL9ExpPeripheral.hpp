
// #pragma once
// #ifndef DIRECTED_DEVO_AVIDAGPL9_EXP_PERIPHERAL_HPP_INCLUDE
// #define DIRECTED_DEVO_AVIDAGPL9_EXP_PERIPHERAL_HPP_INCLUDE

// #include "emp/hardware/AvidaGP.hpp"
// #include "emp/hardware/AvidaCPU_InstLib.hpp"

// #include "../DirectedDevoConfig.hpp"
// #include "../BasePeripheral.hpp"

// namespace dirdevo {

// /// NOTE - this may not be necessary because instruction library needs to live at the world level (to have access to the appropriate task)
// /// Stores peripheral components necessary to run the AvidaGPL9 experiment.
// class AvidaGPL9ExpPeripheral : public BasePeripheral {

// public:
//   using this_t = AvidaGPL9ExpPeripheral;
//   using base_t = BasePeripheral;
//   using hardware_t = emp::AvidaGP;
//   using inst_lib_t = typename hardware_t::inst_lib_t;
//   using config_t = typename base_t::config_t;

// protected:

// public:
//   AvidaGPL9ExpPeripheral() {
//   }

//   void Setup(const config_t& cfg) {
//     // --- do any parameterization here ---
//   }



// };

// AvidaGPL9ExpPeripheral::SetupInstLib() {
//   // todo - add instuction set
// }

// } // namespace dirdevo

// #endif // #ifndef DIRDEVO_UTILITY_EMPTYTYPE_HPP_INCLUDE
