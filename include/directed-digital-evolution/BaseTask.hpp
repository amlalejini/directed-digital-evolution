#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_BASE_TASK_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_BASE_TASK_HPP_INCLUDE

#include <cstddef>

namespace dirdevo {

template<typename DERIVED_T, typename ORG_T>
class BaseTask {

public:

  using org_t = ORG_T;

protected:
public:

  // --- ORGANISM HOOKS ---
  // OnBeforeRepro()
  // OnOffspringReady(DERIVED_T&)
  // OnPlacement(size_t)
  // OnBirth(DERIVED_T&)
  // OnOrgProcessStep

};


} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_BASE_TASK_HPP_INCLUDE