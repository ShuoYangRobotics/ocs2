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

#include <gtest/gtest.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>

#include "ocs2_ipm/IpmSolver.h"

#include <ocs2_core/constraint/LinearStateConstraint.h>
#include <ocs2_core/constraint/LinearStateInputConstraint.h>
#include <ocs2_core/initialization/DefaultInitializer.h>
#include <ocs2_oc/test/EXP1.h>

using namespace ocs2;

class EXP1_MixedStateInputIneqConstraints final : public StateInputConstraint {
 public:
  EXP1_MixedStateInputIneqConstraints(scalar_t xumin, scalar_t xumax)
      : StateInputConstraint(ConstraintOrder::Linear), xumin_(xumin), xumax_(xumax) {}
  ~EXP1_MixedStateInputIneqConstraints() override = default;

  EXP1_MixedStateInputIneqConstraints* clone() const override { return new EXP1_MixedStateInputIneqConstraints(*this); }

  size_t getNumConstraints(scalar_t time) const override { return 2; }

  vector_t getValue(scalar_t t, const vector_t& x, const vector_t& u, const PreComputation&) const override {
    vector_t e(2);
    e << (x(0) * u(0) - xumin_), (xumax_ - x(1) * u(0));
    return e;
  }

  VectorFunctionLinearApproximation getLinearApproximation(scalar_t t, const vector_t& x, const vector_t& u,
                                                           const PreComputation& preComp) const override {
    VectorFunctionLinearApproximation e(2, x.size(), u.size());
    e.f = getValue(t, x, u, preComp);
    e.dfdx << u(0), 0, 0, -u(0);
    e.dfdu << x(0), -x(1);
    return e;
  }

 private:
  const scalar_t xumin_, xumax_;
};

TEST(Exp1Test, Unconstrained) {
  constexpr size_t STATE_DIM = 2;
  constexpr size_t INPUT_DIM = 1;

  // Solver settings
  const auto settings = []() {
    ipm::Settings s;
    s.dt = 0.01;
    s.ipmIteration = 20;
    s.useFeedbackPolicy = true;
    s.printSolverStatistics = true;
    s.printSolverStatus = true;
    s.printLinesearch = true;
    s.printSolverStatistics = true;
    s.printSolverStatus = true;
    s.printLinesearch = true;
    s.nThreads = 1;
    s.initialBarrierParameter = 1.0e-02;
    s.targetBarrierParameter = 1.0e-04;
    s.barrierLinearDecreaseFactor = 0.2;
    s.barrierSuperlinearDecreasePower = 1.5;
    s.fractionToBoundaryMargin = 0.995;
    return s;
  }();

  const scalar_array_t initEventTimes{0.2262, 1.0176};
  const size_array_t modeSequence{0, 1, 2};
  auto referenceManagerPtr = getExp1ReferenceManager(initEventTimes, modeSequence);
  auto problem = createExp1Problem(referenceManagerPtr);

  const scalar_t startTime = 0.0;
  const scalar_t finalTime = 3.0;
  const vector_t initState = (vector_t(STATE_DIM) << 2.0, 3.0).finished();

  DefaultInitializer zeroInitializer(INPUT_DIM);

  // Solve
  IpmSolver solver(settings, problem, zeroInitializer);
  solver.setReferenceManager(referenceManagerPtr);
  solver.run(startTime, initState, finalTime);
}

TEST(Exp1Test, Constrained) {
  constexpr size_t STATE_DIM = 2;
  constexpr size_t INPUT_DIM = 1;

  auto getStateBoxConstraint = [&](const vector_t& minState, const vector_t& maxState) {
    constexpr size_t numIneqConstraint = 2 * STATE_DIM;
    const vector_t e = (vector_t(numIneqConstraint) << -minState, maxState).finished();
    const matrix_t C =
        (matrix_t(numIneqConstraint, STATE_DIM) << matrix_t::Identity(STATE_DIM, STATE_DIM), -matrix_t::Identity(STATE_DIM, STATE_DIM))
            .finished();
    return std::make_unique<LinearStateConstraint>(std::move(e), std::move(C));
  };

  auto getInputBoxConstraint = [&](scalar_t minInput, scalar_t maxInput) {
    constexpr size_t numIneqConstraint = 2 * INPUT_DIM;
    const vector_t e = (vector_t(numIneqConstraint) << -minInput, maxInput).finished();
    const matrix_t C = matrix_t::Zero(numIneqConstraint, STATE_DIM);
    const matrix_t D =
        (matrix_t(numIneqConstraint, INPUT_DIM) << matrix_t::Identity(INPUT_DIM, INPUT_DIM), -matrix_t::Identity(INPUT_DIM, INPUT_DIM))
            .finished();
    return std::make_unique<LinearStateInputConstraint>(std::move(e), std::move(C), std::move(D));
  };

  // Solver settings
  const auto settings = []() {
    ipm::Settings s;
    s.dt = 0.01;
    s.ipmIteration = 20;
    s.useFeedbackPolicy = true;
    s.printSolverStatistics = true;
    s.printSolverStatus = true;
    s.printLinesearch = true;
    s.printSolverStatistics = true;
    s.printSolverStatus = true;
    s.printLinesearch = true;
    s.nThreads = 1;
    s.initialBarrierParameter = 1.0e-02;
    s.targetBarrierParameter = 1.0e-04;
    s.barrierLinearDecreaseFactor = 0.2;
    s.barrierSuperlinearDecreasePower = 1.5;
    s.fractionToBoundaryMargin = 0.995;
    return s;
  }();

  const scalar_array_t initEventTimes{0.2262, 1.0176};
  const size_array_t modeSequence{0, 1, 2};
  auto referenceManagerPtr = getExp1ReferenceManager(initEventTimes, modeSequence);
  auto problem = createExp1Problem(referenceManagerPtr);

  // add inequality constraints
  const scalar_t umin = -1.0;
  const scalar_t umax = 1.0;
  problem.inequalityConstraintPtr->add("ubound", getInputBoxConstraint(umin, umax));
  const vector_t xmin = (vector_t(2) << -0.0, -0.0).finished();
  const vector_t xmax = (vector_t(2) << 3.0, 4.0).finished();
  problem.stateInequalityConstraintPtr->add("xbound", getStateBoxConstraint(xmin, xmax));
  problem.finalInequalityConstraintPtr->add("xbound", getStateBoxConstraint(xmin, xmax));

  const scalar_t startTime = 0.0;
  const scalar_t finalTime = 3.0;
  const vector_t initState = (vector_t(STATE_DIM) << 2.0, 3.0).finished();

  DefaultInitializer zeroInitializer(INPUT_DIM);

  // Solve
  IpmSolver solver(settings, problem, zeroInitializer);
  solver.setReferenceManager(referenceManagerPtr);
  solver.run(startTime, initState, finalTime);

  const auto primalSolution = solver.primalSolution(finalTime);

  // check constraint satisfaction
  for (const auto& x : primalSolution.stateTrajectory_) {
    if (x.size() > 0) {
      ASSERT_TRUE((x - xmin).minCoeff() >= 0);
      ASSERT_TRUE((xmax - x).minCoeff() >= 0);
    }
  }
  for (const auto& u : primalSolution.inputTrajectory_) {
    if (u.size() > 0) {
      ASSERT_TRUE(u(0) - umin >= 0);
      ASSERT_TRUE(umax - u(0) >= 0);
    }
  }

  // solve with shifted horizon
  const scalar_array_t shiftTime = {0.05, 0.1, 0.3, 0.5, 0.8, 0.12, 0.16, 0.19};
  for (const auto e : shiftTime) {
    solver.run(startTime + e, initState, finalTime + e);
  }
}

TEST(Exp1Test, MixedConstrained) {
  constexpr size_t STATE_DIM = 2;
  constexpr size_t INPUT_DIM = 1;

  // Solver settings
  const auto settings = []() {
    ipm::Settings s;
    s.dt = 0.01;
    s.ipmIteration = 20;
    s.useFeedbackPolicy = true;
    s.printSolverStatistics = true;
    s.printSolverStatus = true;
    s.printLinesearch = true;
    s.printSolverStatistics = true;
    s.printSolverStatus = true;
    s.printLinesearch = true;
    s.nThreads = 1;
    s.initialBarrierParameter = 1.0e-02;
    s.targetBarrierParameter = 1.0e-04;
    s.barrierLinearDecreaseFactor = 0.2;
    s.barrierSuperlinearDecreasePower = 1.5;
    s.fractionToBoundaryMargin = 0.995;
    return s;
  }();

  const scalar_array_t initEventTimes{0.2262, 1.0176};
  const size_array_t modeSequence{0, 1, 2};
  auto referenceManagerPtr = getExp1ReferenceManager(initEventTimes, modeSequence);
  auto problem = createExp1Problem(referenceManagerPtr);

  // add inequality constraints
  const scalar_t xumin = -1.0;
  const scalar_t xumax = 1.0;
  auto stateInputIneqConstraint = std::make_unique<EXP1_MixedStateInputIneqConstraints>(xumin, xumax);
  auto stateInputIneqConstraintCloned = stateInputIneqConstraint->clone();
  problem.inequalityConstraintPtr->add("bound", std::move(stateInputIneqConstraint));
  const scalar_t startTime = 0.0;
  const scalar_t finalTime = 3.0;
  const vector_t initState = (vector_t(STATE_DIM) << 2.0, 3.0).finished();

  DefaultInitializer zeroInitializer(INPUT_DIM);

  // Solve
  IpmSolver solver(settings, problem, zeroInitializer);
  solver.setReferenceManager(referenceManagerPtr);
  solver.run(startTime, initState, finalTime);

  const auto primalSolution = solver.primalSolution(finalTime);

  // check constraint satisfaction
  const size_t N = primalSolution.inputTrajectory_.size();
  for (size_t i = 0; i < N; ++i) {
    const auto t = primalSolution.timeTrajectory_[i];
    const auto& x = primalSolution.stateTrajectory_[i];
    const auto& u = primalSolution.inputTrajectory_[i];
    const auto constraintValue = stateInputIneqConstraintCloned->getValue(t, x, u, PreComputation());
    ASSERT_TRUE(constraintValue.minCoeff() >= 0.0);
  }

  // solve with shifted horizon
  const scalar_array_t shiftTime = {0.05, 0.1, 0.3, 0.5, 0.8, 0.12, 0.16, 0.19};
  for (const auto e : shiftTime) {
    solver.run(startTime + e, initState, finalTime + e);
  }
}
