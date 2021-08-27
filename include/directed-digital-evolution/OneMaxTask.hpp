#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONE_MAX_TASK_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_ONE_MAX_TASK_HPP_INCLUDE

#include "BaseTask.hpp"

namespace dirdevo {

template<typename ORG_T>
class OneMaxTask : BaseTask<OneMaxTask<ORG_T>, ORG_T> {

public:

  using org_t = ORG_T;
  using this_t = OneMaxTask<org_t>;
  using base_t = BaseTask<this_t,org_t>;

protected:
public:

  // --- ORGANISM HOOKS ---
  // OnBeforeRepro()
  // OnOffspringReady(DERIVED_T&)
  // OnPlacement(size_t)
  // OnBirth(DERIVED_T&)

};


} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONE_MAX_TASK_HPP_INCLUDE