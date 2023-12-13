/******************************************************************************
Copyright (c) 2017, Farbod Farshidian. All rights reserved.

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

#pragma once

#include <ocs2_core/Types.h>
#include <ocs2_core/model_data/Metrics.h>
#include <ocs2_core/model_data/ModelData.h>
#include <ocs2_core/model_data/Multiplier.h>

#include <ocs2_oc/oc_problem/OptimalControlProblem.h>

namespace ocs2 {

/**
 * Calculates an LQ approximate of the constrained optimal control problem at a given time, state, and input.
 *
 * @param [in] problem: The optimal control problem
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @param [in] input: The current input.
 * @param [in] multipliers: The current multipliers associated to the equality and inequality Lagrangians.
 * @param [out] modelData: The output data model.
 */
void approximateIntermediateLQ(OptimalControlProblem& problem, const scalar_t time, const vector_t& state, const vector_t& input,
                               const MultiplierCollection& multipliers, ModelData& modelData);

/**
 * Calculates an LQ approximate of the constrained optimal control problem at a given time, state, and input.
 *
 * @param [in] problem: The optimal control problem
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @param [in] input: The current input.
 * @param [in] multipliers: The current multipliers associated to the equality and inequality Lagrangians.
 * @return The output data model.
 */
inline ModelData approximateIntermediateLQ(OptimalControlProblem& problem, const scalar_t time, const vector_t& state,
                                           const vector_t& input, const MultiplierCollection& multipliers) {
  ModelData md;
  approximateIntermediateLQ(problem, time, state, input, multipliers, md);
  return md;
}

/**
 * Calculates an LQ approximate of the constrained optimal control problem at a jump event time.
 *
 * @param [in] problem: The optimal control problem
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @param [in] multipliers: The current multipliers associated to the equality and inequality Lagrangians.
 * @param [out] modelData: The output data model.
 */
void approximatePreJumpLQ(OptimalControlProblem& problem, const scalar_t& time, const vector_t& state,
                          const MultiplierCollection& multipliers, ModelData& modelData);

/**
 * Calculates an LQ approximate of the constrained optimal control problem at a jump event time.
 *
 * @param [in] problem: The optimal control problem
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @param [in] multipliers: The current multipliers associated to the equality and inequality Lagrangians.
 * @return The output data model.
 */
inline ModelData approximatePreJumpLQ(OptimalControlProblem& problem, const scalar_t& time, const vector_t& state,
                                      const MultiplierCollection& multipliers) {
  ModelData md;
  approximatePreJumpLQ(problem, time, state, multipliers, md);
  return md;
}

/**
 * Calculates an LQ approximate of the constrained optimal control problem at final time.
 *
 * @param [in] problem: The optimal control problem
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @param [in] multipliers: The current multipliers associated to the equality and inequality Lagrangians.
 * @param [out] modelData: The output data model.
 */
void approximateFinalLQ(OptimalControlProblem& problem, const scalar_t& time, const vector_t& state,
                        const MultiplierCollection& multipliers, ModelData& modelData);

/**
 * Calculates an LQ approximate of the constrained optimal control problem at final time.
 *
 * @param [in] problem: The optimal control problem
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @param [in] multipliers: The current multipliers associated to the equality and inequality Lagrangians.
 * @return The output data model.
 */
inline ModelData approximateFinalLQ(OptimalControlProblem& problem, const scalar_t& time, const vector_t& state,
                                    const MultiplierCollection& multipliers) {
  ModelData md;
  approximateFinalLQ(problem, time, state, multipliers, md);
  return md;
}

/**
 * Compute the total intermediate cost (i.e. cost + softConstraints). It is assumed that the precomputation request is already made.
 */
scalar_t computeCost(const OptimalControlProblem& problem, const scalar_t& time, const vector_t& state, const vector_t& input);

/**
 * Compute the quadratic approximation of the total intermediate cost (i.e. cost + softConstraints). It is assumed that the precomputation
 * request is already made.
 */
ScalarFunctionQuadraticApproximation approximateCost(const OptimalControlProblem& problem, const scalar_t& time, const vector_t& state,
                                                     const vector_t& input);

/**
 * Compute the total preJump cost (i.e. cost + softConstraints). It is assumed that the precomputation request is already made.
 */
scalar_t computeEventCost(const OptimalControlProblem& problem, const scalar_t& time, const vector_t& state);

/**
 * Compute the quadratic approximation of the total preJump cost (i.e. cost + softConstraints). It is assumed that the precomputation
 * request is already made.
 */
ScalarFunctionQuadraticApproximation approximateEventCost(const OptimalControlProblem& problem, const scalar_t& time,
                                                          const vector_t& state);

/**
 * Compute the total final cost (i.e. cost + softConstraints). It is assumed that the precomputation request is already made.
 */
scalar_t computeFinalCost(const OptimalControlProblem& problem, const scalar_t& time, const vector_t& state);

/**
 * Compute the quadratic approximation of the total final cost (i.e. cost + softConstraints). It is assumed that the precomputation
 * request is already made.
 */
ScalarFunctionQuadraticApproximation approximateFinalCost(const OptimalControlProblem& problem, const scalar_t& time,
                                                          const vector_t& state);

/**
 * Compute the intermediate-time Metrics (i.e. cost, softConstraints, and constraints).
 *
 * @note It is assumed that the precomputation request is already made.
 * problem.preComputationPtr->request(Request::Cost + Request::Constraint + Request::SoftConstraint, t, x, u)
 *
 * @param [in] problem: The optimal control probelm
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @param [in] input: The current input.
 * @param [in] dynamicsViolation: The violation of dynamics. It depends on the transcription method.
 * @return The output Metrics.
 */
Metrics computeIntermediateMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state, const vector_t& input,
                                   vector_t&& dynamicsViolation = vector_t());

/**
 * Compute the intermediate-time Metrics (i.e. cost, softConstraints, and constraints).
 *
 * @note It is assumed that the precomputation request is already made.
 * problem.preComputationPtr->request(Request::Cost + Request::Constraint + Request::SoftConstraint, t, x, u)
 *
 * @param [in] problem: The optimal control probelm
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @param [in] input: The current input.
 * @param [in] multipliers: The current multipliers associated to the equality and inequality Lagrangians.
 * @param [in] dynamicsViolation: The violation of dynamics. It depends on the transcription method.
 * @return The output Metrics.
 */
Metrics computeIntermediateMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state, const vector_t& input,
                                   const MultiplierCollection& multipliers, vector_t&& dynamicsViolation = vector_t());

/**
 * Compute the event-time Metrics based on pre-jump state value (i.e. cost, softConstraints, and constraints).
 *
 * @note It is assumed that the precomputation request is already made.
 * problem.preComputationPtr->requestPreJump(Request::Cost + Request::Constraint + Request::SoftConstraint, t, x)
 *
 * @param [in] problem: The optimal control probelm
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @param [in] dynamicsViolation: The violation of dynamics. It depends on the transcription method.
 * @return The output Metrics.
 */
Metrics computePreJumpMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state,
                              vector_t&& dynamicsViolation = vector_t());

/**
 * Compute the event-time Metrics based on pre-jump state value (i.e. cost, softConstraints, and constraints).
 *
 * @note It is assumed that the precomputation request is already made.
 * problem.preComputationPtr->requestPreJump(Request::Cost + Request::Constraint + Request::SoftConstraint, t, x)
 *
 * @param [in] problem: The optimal control probelm
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @param [in] multipliers: The current multipliers associated to the equality and inequality Lagrangians.
 * @param [in] dynamicsViolation: The violation of dynamics. It depends on the transcription method.
 * @return The output Metrics.
 */
Metrics computePreJumpMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state,
                              const MultiplierCollection& multipliers, vector_t&& dynamicsViolation = vector_t());

/**
 * Compute the final-time Metrics (i.e. cost, softConstraints, and constraints).
 *
 * @note It is assumed that the precomputation request is already made.
 * problem.preComputationPtr->requestFinal(Request::Cost + Request::Constraint + Request::SoftConstraint, t, x)
 *
 * @param [in] problem: The optimal control probelm
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @return The output Metrics.
 */
Metrics computeFinalMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state);

/**
 * Compute the final-time Metrics (i.e. cost, softConstraints, and constraints).
 *
 * @note It is assumed that the precomputation request is already made.
 * problem.preComputationPtr->requestFinal(Request::Cost + Request::Constraint + Request::SoftConstraint, t, x)
 *
 * @param [in] problem: The optimal control probelm
 * @param [in] time: The current time.
 * @param [in] state: The current state.
 * @param [in] multipliers: The current multipliers associated to the equality and inequality Lagrangians.
 * @return The output Metrics.
 */
Metrics computeFinalMetrics(OptimalControlProblem& problem, const scalar_t time, const vector_t& state,
                            const MultiplierCollection& multipliers);

}  // namespace ocs2
