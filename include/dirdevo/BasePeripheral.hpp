#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_BASE_PERIPHERAL_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_BASE_PERIPHERAL_HPP_INCLUDE

#include <cstddef>
#include <functional>

#include "emp/base/vector.hpp"

#include "DirectedDevoConfig.hpp"

namespace dirdevo {

class BasePeripheral {
public:
  using config_t = DirectedDevoConfig;
protected:
public:

  virtual void Setup(const config_t& cfg) { }

};

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_BASE_PERIPHERAL_HPP_INCLUDE