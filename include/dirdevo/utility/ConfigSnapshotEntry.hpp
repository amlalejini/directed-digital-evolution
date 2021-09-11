#pragma once

#include <string>

namespace dirdevo {

  /// Useful struct that experiment components use to pass entries along to be snapshotted by the experiment
  struct ConfigSnapshotEntry {

    std::string param;    ///< Parameter name
    std::string value;    ///< Parameter value
    std::string source;   ///< Parameter source

    ConfigSnapshotEntry(
      const std::string& p,
      const std::string& v,
      const std::string& s
    ) :
      param(p),
      value(v),
      source(s)
    { }

  };

}