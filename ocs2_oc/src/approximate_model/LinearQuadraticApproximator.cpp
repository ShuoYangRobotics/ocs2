/******************************************************************************
Copyright (c) 2020, Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#include <iostream>

#include "ocs2_oc/approximate_model/LinearQuadraticApproximator.h"

#include <ocs2_core/misc/LinearAlgebra.h>

namespace ocs2 {

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void approximateIntermediateLQ(OptimalControlProblem& problem, const scalar_t time, const vector_t& state, const vector_t& input,
                               const MultiplierCollection& multipliers, ModelData& modelData) {
  auto& preComputation = *problem.preComputationPtr;
  constexpr auto request = Request::Cost + Request::SoftConstraint + Request::Constraint + Request::Dynamics + Request::Approximation;
  preComputation.request(request, time, state, input);

  modelData.time = time;
  modelData.stateDim = state.rows();
  modelData.inputDim = input.rows();

  // Dynamics
  modelData.dynamicsCovariance = problem.dynamicsPtr->dynamicsCovariance(time, state, input);
  modelData.dynamics = problem.dynamicsPtr->linearApproximation(time, state, input, preComputation);
  modelData.dynamicsBias.setZero(modelData.dynamics.dfdx.rows());

  // Cost
  modelData.cost = ocs2::approximateCost(problem, time, state, input);

  // Equality constraints
  modelData.stateEqConstraint = problem.stateEqualityConstraintPtr->getLinearApproximation(time, state, preComputation);
  modelData.stateInputEqConstraint = problem.equalityConstraintPtr->getLinearApproximation(time, state, input, preComputation);

  // Lagrangians
  if (!problem.stateEqualityLagrangianPtr->empty()) {
    auto approx = problem.stateEqualityLagrangianPtr->getQuadraticApproximation(time, state, multipliers.stateEq, preComputation);
    modelData.cost.f += approx.f;
    modelData.cost.dfdx += approx.dfdx;
    modelData.cost.dfdxx += approx.dfdxx;
  }
  if (!problem.stateInequalityLagrangianPtr->empty()) {
    auto approx = problem.stateInequalityLagrangianPtr->getQuadraticApproximation(time, state, multipliers.stateIneq, preComputation);
    modelData.cost.f += approx.f;
    modelData.cost.dfdx += approx.dfdx;
    modelData.cost.dfdxx += approx.dfdxx;
  }
  if (!problem.equalityLagrangianPtr->empty()) {
    modelData.cost +=
        problem.equalityLagrangianPtr->getQuadraticApproximation(time, state, input, multipliers.stateInputEq, preComputation);
  }
  if (!problem.inequalityLagrangianPtr->empty()) {
    modelData.cost +=
        problem.inequalityLagrangianPtr->getQuadraticApproximation(time, state, input, multipliers.stateInputIneq, preComputation);
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void approximatePreJumpLQ(OptimalControlProblem& problem, const scalar_t& time, const vector_t& state,
                          const MultiplierCollection& multipliers, ModelData& modelData) {
  auto& preComputation = *problem.preComputationPtr;
  constexpr auto request = Request::Cost + Request::SoftConstraint + Request::Constraint + Request::Dynamics + Request::Approximation;
  preComputation.requestPreJump(request, time, state);

  modelData.time = time;
  modelData.stateDim = state.rows();
  modelData.inputDim = 0;

  // Jump map
  modelData.dynamics = problem.dynamicsPtr->jumpMapLinearApproximation(time, state, preComputation);
  modelData.dynamicsBias.setZero(modelData.dynamics.dfdx.rows());

  // Pre-jump cost
  modelData.cost = approximateEventCost(problem, time, state);

  // state equality constraint
  modelData.stateEqConstraint = problem.preJumpEqualityConstraintPtr->getLinearApproximation(time, state, preComputation);

  // Lagrangians
  if (!problem.preJumpEqualityLagrangianPtr->empty()) {
    auto approx = problem.preJumpEqualityLagrangianPtr->getQuadraticApproximation(time, state, multipliers.stateEq, preComputation);
    modelData.cost.f += approx.f;
    modelData.cost.dfdx += approx.dfdx;
    modelData.cost.dfdxx += approx.dfdxx;
  }
  if (!problem.preJumpInequalityLagrangianPtr->empty()) {
    auto approx = problem.preJumpInequalityLagrangianPtr->getQuadraticApproximation(time, state, multipliers.stateIneq, preComputation);
    modelData.cost.f += approx.f;
    modelData.cost.dfdx += approx.dfdx;
    modelData.cost.dfdxx += approx.dfdxx;
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void approximateFinalLQ(OptimalControlProblem& problem, const scalar_t& time, const vector_t& state,
                        const MultiplierCollection& multipliers, ModelData& modelData) {
  auto& preComputation = *problem.preComputationPtr;
  constexpr auto request = Request::Cost + Request::SoftConstraint + Request::Constraint + Request::Approximation;
  preComputation.requestFinal(request, time, state);

  modelData.time = time;
  modelData.stateDim = state.rows();
  modelData.inputDim = 0;
  modelData.dynamicsBias = vector_t();

  // Dynamics
  modelData.dynamics = VectorFunctionLinearApproximation();

  // state equality constraint
  modelData.stateEqConstraint = problem.finalEqualityConstraintPtr->getLinearApproximation(time, state, preComputation);

  // Final cost
  modelData.cost = approximateFinalCost(problem, time, state);

  // Lagrangians
  if (!problem.finalEqualityLagrangianPtr->empty()) {
    auto approx = problem.finalEqualityLagrangianPtr->getQuadraticApproximation(time, state, multipliers.stateEq, preComputation);
    modelData.cost.f += approx.f;
    modelData.cost.dfdx += approx.dfdx;
    modelData.cost.dfdxx += approx.dfdxx;
  }
  if (!problem.finalInequalityLagrangianPtr->empty()) {
    auto approx = problem.finalInequalityLagrangianPtr->getQuadraticApproximation(time, state, multipliers.stateIneq, preComputation);
    modelData.cost.f += approx.f;
    modelData.cost.dfdx += approx.dfdx;
    modelData.cost.dfdxx += approx.dfdxx;
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
scalar_t computeCost(const OptimalControlProblem& problem, const scalar_t& time, const vector_t& state, const vector_t& input) {
  const auto& targetTrajectories = *problem.targetTrajectoriesPtr;
  const auto& preComputation = *problem.preComputationPtr;

  // Compute and sum all costs
  auto cost = problem.costPtr->getValue(time, state, input, targetTrajectories, preComputation);
  cost += problem.softConstraintPtr->getValue(time, state, input, targetTrajectories, preComputation);
  cost += problem.stateCostPtr->getValue(time, state, targetTrajectories, preComputation);
  cost += problem.stateSoftConstraintPtr->getValue(time, state, targetTrajectories, preComputation);

  return cost;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
ScalarFunctionQuadraticApproximation approximateCost(const OptimalControlProblem& problem, const scalar_t& time, const vector_t& state,
                                                     const vector_t& input) {
  const auto& targetTrajectories = *problem.targetTrajectoriesPtr;
  const auto& preComputation = *problem.preComputationPtr;

  // get the state-input cost approximations
  auto cost = problem.costPtr->getQuadraticApproximation(time, state, input, targetTrajectories, preComputation);

  if (!problem.softConstraintPtr->empty()) {
    cost += problem.softConstraintPtr->getQuadraticApproximation(time, state, input, targetTrajectories, preComputation);
  }

  // get the state only cost approximations
  if (!problem.stateCostPtr->empty()) {
    auto stateCost = problem.stateCostPtr->getQuadraticApproximation(time, state, targetTrajectories, preComputation);
    cost.f += stateCost.f;
    cost.dfdx += stateCost.dfdx;
    cost.dfdxx += stateCost.dfdxx;
  }

  if (!problem.stateSoftConstraintPtr->empty()) {
    auto stateCost = problem.stateSoftConstraintPtr->getQuadraticApproximation(time, state, targetTrajectories, preComputation);
    cost.f += stateCost.f;
    cost.dfdx += stateCost.dfdx;
    cost.dfdxx += stateCost.dfdxx;
  }

  return cost;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
scalar_t computeEventCost(const OptimalControlProblem& problem, const scalar_t& time, const vector_t& state) {
  const auto& targetTrajectories = *problem.targetTrajectoriesPtr;
  const auto& preComputation = *problem.preComputationPtr;

  auto cost = problem.preJumpCostPtr->getValue(time, state, targetTrajectories, preComputation);
  cost += problem.preJumpSoftConstraintPtr->getValue(time, state, targetTrajectories, preComputation);

  return cost;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
ScalarFunctionQuadraticApproximation approximateEventCost(const OptimalControlProblem& problem, const scalar_t& time,
                                                          const vector_t& state) {
  const auto& targetTrajectories = *problem.targetTrajectoriesPtr;
  const auto& preComputation = *problem.preComputationPtr;

  auto cost = problem.preJumpCostPtr->getQuadraticApproximation(time, state, targetTrajectories, preComputation);
  if (!problem.preJumpSoftConstraintPtr->empty()) {
    cost += problem.preJumpSoftConstraintPtr->getQuadraticApproximation(time, state, targetTrajectories, preComputation);
  }

  return cost;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
scalar_t computeFinalCost(const OptimalControlProblem& problem, const scalar_t& time, const vector_t& state) {
  const auto& targetTrajectories = *problem.targetTrajectoriesPtr;
  const auto& preComputation = *problem.preComputationPtr;

  auto cost = problem.finalCostPtr->getValue(time, state, targetTrajectories, preComputation);
  cost += problem.finalSoftConstraintPtr->getValue(time, state, targetTrajectories, preComputation);

  return cost;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
ScalarFunctionQuadraticApproximation approximateFinalCost(const OptimalControlProblem& problem, const scalar_t& time,
                                                          const vector_t& state) {
  const auto& targetTrajectories = *problem.targetTrajectoriesPtr;
  const auto& preComputation = *problem.preComputationPtr;

  auto cost = problem.finalCostPtr->getQuadraticApproximation(time, state, targetTrajectories, preComputation);
  if (!problem.finalSoftConstraintPtr->empty()) {
    cost += problem.finalSoftConstraintPtr->getQuadraticApproximation(time, state, targetTrajectories, preComputation);
  }

  return cost;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
Metrics computeIntermediateMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state, const vector_t& input,
                                   vector_t&& dynamicsViolation) {
  auto& preComputation = *problem.preComputationPtr;

  Metrics metrics;

  // Cost
  metrics.cost = computeCost(problem, time, state, input);

  // Dynamics violation
  metrics.dynamicsViolation = std::move(dynamicsViolation);

  // Equality constraints
  if (!problem.stateEqualityConstraintPtr->empty()) {
    metrics.stateEqConstraint = problem.stateEqualityConstraintPtr->getValue(time, state, preComputation);
  }
  if (!problem.equalityConstraintPtr->empty()) {
    metrics.stateInputEqConstraint = problem.equalityConstraintPtr->getValue(time, state, input, preComputation);
  }

  // Inequality constraints
  if (!problem.stateInequalityConstraintPtr->empty()) {
    metrics.stateIneqConstraint = problem.stateInequalityConstraintPtr->getValue(time, state, preComputation);
  }
  if (!problem.inequalityConstraintPtr->empty()) {
    metrics.stateInputIneqConstraint = problem.inequalityConstraintPtr->getValue(time, state, input, preComputation);
  }

  return metrics;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
Metrics computeIntermediateMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state, const vector_t& input,
                                   const MultiplierCollection& multipliers, vector_t&& dynamicsViolation) {
  auto& preComputation = *problem.preComputationPtr;

  // cost, dynamics violation, equlaity constraints, inequlaity constraints
  auto metrics = computeIntermediateMetrics(problem, time, state, input, std::move(dynamicsViolation));

  // Equality Lagrangians
  metrics.stateEqLagrangian = problem.stateEqualityLagrangianPtr->getValue(time, state, multipliers.stateEq, preComputation);
  metrics.stateInputEqLagrangian = problem.equalityLagrangianPtr->getValue(time, state, input, multipliers.stateInputEq, preComputation);

  // Inequality Lagrangians
  metrics.stateIneqLagrangian = problem.stateInequalityLagrangianPtr->getValue(time, state, multipliers.stateIneq, preComputation);
  metrics.stateInputIneqLagrangian =
      problem.inequalityLagrangianPtr->getValue(time, state, input, multipliers.stateInputIneq, preComputation);

  return metrics;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
Metrics computePreJumpMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state, vector_t&& dynamicsViolation) {
  auto& preComputation = *problem.preComputationPtr;

  Metrics metrics;

  // Cost
  metrics.cost = computeEventCost(problem, time, state);

  // Dynamics violation
  metrics.dynamicsViolation = std::move(dynamicsViolation);

  // Equality constraint
  if (!problem.preJumpEqualityConstraintPtr->empty()) {
    metrics.stateEqConstraint = problem.preJumpEqualityConstraintPtr->getValue(time, state, preComputation);
  }

  // Inequality constraint
  if (!problem.preJumpInequalityConstraintPtr->empty()) {
    metrics.stateIneqConstraint = problem.preJumpInequalityConstraintPtr->getValue(time, state, preComputation);
  }

  return metrics;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
Metrics computePreJumpMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state,
                              const MultiplierCollection& multipliers, vector_t&& dynamicsViolation) {
  auto& preComputation = *problem.preComputationPtr;

  // cost, dynamics violation, equlaity constraints, inequlaity constraints
  auto metrics = computePreJumpMetrics(problem, time, state, std::move(dynamicsViolation));

  // Equality Lagrangians
  metrics.stateEqLagrangian = problem.preJumpEqualityLagrangianPtr->getValue(time, state, multipliers.stateEq, preComputation);

  // Inequality Lagrangians
  metrics.stateIneqLagrangian = problem.preJumpInequalityLagrangianPtr->getValue(time, state, multipliers.stateIneq, preComputation);

  return metrics;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
Metrics computeFinalMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state) {
  auto& preComputation = *problem.preComputationPtr;

  Metrics metrics;

  // Cost
  metrics.cost = computeFinalCost(problem, time, state);

  // Dynamics violation
  // metrics.dynamicsViolation = vector_t();

  // Equality constraint
  if (!problem.finalEqualityConstraintPtr->empty()) {
    metrics.stateEqConstraint = problem.finalEqualityConstraintPtr->getValue(time, state, preComputation);
  }

  // Inequality constraint
  if (!problem.finalInequalityConstraintPtr->empty()) {
    metrics.stateIneqConstraint = problem.finalInequalityConstraintPtr->getValue(time, state, preComputation);
  }

  return metrics;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
Metrics computeFinalMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state,
                            const MultiplierCollection& multipliers) {
  auto& preComputation = *problem.preComputationPtr;

  // cost, equlaity constraints, inequlaity constraints
  auto metrics = computeFinalMetrics(problem, time, state);

  // Equality Lagrangians
  metrics.stateEqLagrangian = problem.finalEqualityLagrangianPtr->getValue(time, state, multipliers.stateEq, preComputation);

  // Inequality Lagrangians
  metrics.stateIneqLagrangian = problem.finalInequalityLagrangianPtr->getValue(time, state, multipliers.stateIneq, preComputation);

  return metrics;
}

}  // namespace ocs2
