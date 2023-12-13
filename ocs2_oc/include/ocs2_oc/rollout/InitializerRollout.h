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

#include <memory>

#include <ocs2_core/initialization/Initializer.h>

#include "ocs2_oc/rollout/RolloutBase.h"

namespace ocs2 {

/**
 * This class is an interface class for forward rollout of the initializer.
 */
class InitializerRollout : public RolloutBase {
 public:
  /**
   * Constructor.
   *
   * @param [in] initializer: The initializer for the state and the input.
   * @param [in] rolloutSettings: The rollout settings.
   */
  explicit InitializerRollout(const Initializer& initializer, rollout::Settings rolloutSettings = rollout::Settings());

  ~InitializerRollout() override = default;
  InitializerRollout* clone() const override;

  vector_t run(scalar_t initTime, const vector_t& initState, scalar_t finalTime, ControllerBase* controller, ModeSchedule& modeSchedule,
               scalar_array_t& timeTrajectory, size_array_t& postEventIndices, vector_array_t& stateTrajectory,
               vector_array_t& inputTrajectory) override;

 private:
  std::unique_ptr<Initializer> initializerPtr_;
};

}  // namespace ocs2
