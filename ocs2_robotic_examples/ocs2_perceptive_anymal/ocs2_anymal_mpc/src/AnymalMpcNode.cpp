/*
 * AnymalMPC.cpp
 *
 *  Created on: Apr 15, 2018
 *      Author: farbod
 */

#include <ocs2_quadruped_interface/QuadrupedMpcNode.h>
#include <ros/init.h>

#include <ocs2_ddp/DDP_Settings.h>
#include <ocs2_mpc/MPC_Settings.h>
#include <ocs2_quadruped_interface/QuadrupedMpc.h>

#include "ocs2_anymal_mpc/AnymalInterface.h"

int main(int argc, char* argv[]) {
  std::vector<std::string> programArgs{};
  ros::removeROSArgs(argc, argv, programArgs);
  if (programArgs.size() < 3) {
    throw std::runtime_error("No robot name and config folder specified. Aborting.");
  }
  const std::string descriptionName(programArgs[1]);
  const std::string configName(programArgs[2]);

  // Initialize ros node
  ros::init(argc, argv, "anymal_mpc");
  ros::NodeHandle nodeHandle;

  std::string urdfString;
  nodeHandle.getParam(descriptionName, urdfString);

  auto anymalInterface = anymal::getAnymalInterface(urdfString, anymal::getConfigFolder(configName));
  const auto mpcSettings = ocs2::mpc::loadSettings(anymal::getTaskFilePath(configName));

  switch (anymalInterface->modelSettings().algorithm_) {
    case switched_model::Algorithm::DDP: {
      const auto ddpSettings = ocs2::ddp::loadSettings(anymal::getTaskFilePath(configName));
      auto mpcPtr = getDdpMpc(*anymalInterface, mpcSettings, ddpSettings);
      quadrupedMpcNode(nodeHandle, *anymalInterface, std::move(mpcPtr));
      break;
    }
    case switched_model::Algorithm::SQP: {
      const auto sqpSettings = ocs2::sqp::loadSettings(anymal::getConfigFolder(configName) + "/multiple_shooting.info");
      auto mpcPtr = getSqpMpc(*anymalInterface, mpcSettings, sqpSettings);
      quadrupedMpcNode(nodeHandle, *anymalInterface, std::move(mpcPtr));
      break;
    }
  }

  return 0;
}
