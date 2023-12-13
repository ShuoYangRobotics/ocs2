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

#include "ocs2_ddp/HessianCorrection.h"

#include <unordered_map>

namespace ocs2 {
namespace hessian_correction {

std::string toString(Strategy strategy) {
  static const std::unordered_map<Strategy, std::string> strategyMap{{Strategy::DIAGONAL_SHIFT, "DIAGONAL_SHIFT"},
                                                                     {Strategy::CHOLESKY_MODIFICATION, "CHOLESKY_MODIFICATION"},
                                                                     {Strategy::EIGENVALUE_MODIFICATION, "EIGENVALUE_MODIFICATION"},
                                                                     {Strategy::GERSHGORIN_MODIFICATION, "GERSHGORIN_MODIFICATION"}};
  return strategyMap.at(strategy);
}

Strategy fromString(const std::string& name) {
  static const std::unordered_map<std::string, Strategy> strategyMap{{"DIAGONAL_SHIFT", Strategy::DIAGONAL_SHIFT},
                                                                     {"CHOLESKY_MODIFICATION", Strategy::CHOLESKY_MODIFICATION},
                                                                     {"EIGENVALUE_MODIFICATION", Strategy::EIGENVALUE_MODIFICATION},
                                                                     {"GERSHGORIN_MODIFICATION", Strategy::GERSHGORIN_MODIFICATION}};
  return strategyMap.at(name);
}

void shiftHessian(Strategy strategy, matrix_t& matrix, scalar_t minEigenvalue) {
  assert(matrix.rows() == matrix.cols());
  switch (strategy) {
    case Strategy::DIAGONAL_SHIFT: {
      matrix.diagonal().array() += minEigenvalue;
      break;
    }
    case Strategy::CHOLESKY_MODIFICATION: {
      LinearAlgebra::makePsdCholesky(matrix, minEigenvalue);
      break;
    }
    case Strategy::EIGENVALUE_MODIFICATION: {
      LinearAlgebra::makePsdEigenvalue(matrix, minEigenvalue);
      break;
    }
    case Strategy::GERSHGORIN_MODIFICATION: {
      LinearAlgebra::makePsdGershgorin(matrix, minEigenvalue);
      break;
    }
  }
}

}  // namespace hessian_correction
}  // namespace ocs2
