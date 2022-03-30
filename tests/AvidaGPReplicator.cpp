#define CATCH_CONFIG_MAIN

#include "Catch/single_include/catch2/catch.hpp"

#include <iostream>

#include "emp/math/Random.hpp"

#include "dirdevo/ExperimentSetups/AvidaGP/AvidaGPOrganism.hpp"
#include "dirdevo/ExperimentSetups/AvidaGP/AvidaGPMultiPathwayTask.hpp"
#include "dirdevo/ExperimentSetups/AvidaGP/AvidaGPMutator.hpp"

#include "dirdevo/DirectedDevoWorld.hpp"
#include "dirdevo/DirectedDevoConfig.hpp"


TEST_CASE("AvidaGPReplicator on AvidaGPL9Task", "[l9]") {

  using org_t = dirdevo::AvidaGPOrganism;
  using task_t = dirdevo::AvidaGPMultiPathwayTask;
  using world_t = dirdevo::DirectedDevoWorld<org_t,task_t>;

  // Need to build a world with an AvidaGPL9 task for the correct instruction set.
  dirdevo::DirectedDevoConfig config;
  config.SEED(2);
  config.AVIDAGP_ENV_FILE("example-environment.json");
  emp::Random random(config.SEED());
  world_t world(config, random);

  // Build a replicator using world's task instruction library
  dirdevo::AvidaGPReplicator agp_hardware(world.GetTask().GetInstLib());
  CHECK(agp_hardware.GetSize() == 0);

  SECTION("TEST SELF-REPLICATION INSTRUCTIONS") {

    // Build a test program
    agp_hardware.PushInst("Scope", 0);
    agp_hardware.PushInst("Nop");
    agp_hardware.PushInst("Nop");
    agp_hardware.PushInst("Nop");
    agp_hardware.PushInst("GetLen", 15);
    agp_hardware.PushInst("Countdown", 15, 1);
    agp_hardware.PushInst("CopyInst", 0);
    agp_hardware.PushInst("Scope", 0);
    agp_hardware.PushInst("DivideSelf");

    std::cout << "--- Initial state ---" << std::endl;
    agp_hardware.PrintState();
    for (size_t i = 0; i < 26; ++i) {
      std::cout << "--- " << i << " ---" << std::endl;
      agp_hardware.SingleProcess();
      agp_hardware.PrintState(std::cout);
      std::cout << "SitesCopied:" << agp_hardware.GetSitesCopied();
      std::cout << " IsDoneCopying:" << agp_hardware.IsDoneCopying();
      std::cout << " IsDividing:" << agp_hardware.IsDividing();
      std::cout << " FailedSelfDivisions:" << agp_hardware.GetNumFailedSelfDivisions();
      std::cout << std::endl;
    }
    CHECK(agp_hardware.IsDoneCopying());
    CHECK(agp_hardware.IsDividing());
    CHECK(agp_hardware.GetNumFailedSelfDivisions() == 0);
  }

}