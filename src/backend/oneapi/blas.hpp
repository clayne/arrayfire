/*******************************************************
 * Copyright (c) 2022, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 ********************************************************/

#pragma once
#include <Array.hpp>
#include <math.hpp>

// This file contains the common interface for OneAPI BLAS
// functions

namespace arrayfire {
namespace oneapi {

void initBlas();
void deInitBlas();

template<typename Ti, typename To = Ti>
void gemm(Array<To> &out, af_mat_prop optLhs, af_mat_prop optRhs,
          const To *alpha, const Array<Ti> &lhs, const Array<Ti> &rhs,
          const To *beta);

template<typename T>
Array<T> matmul(const Array<T> &lhs, const Array<T> &rhs, af_mat_prop optLhs,
                af_mat_prop optRhs) {
    int Mdim     = optLhs == AF_MAT_NONE ? 0 : 1;
    int Ndim     = optRhs == AF_MAT_NONE ? 1 : 0;
    Array<T> res = createEmptyArray<T>(
        dim4(lhs.dims()[Mdim], rhs.dims()[Ndim], lhs.dims()[2], lhs.dims()[3]));
    static const T alpha = scalar<T>(1.0);
    static const T beta  = scalar<T>(0.0);
    gemm(res, optLhs, optRhs, &alpha, lhs, rhs, &beta);
    return res;
}

template<typename T>
Array<T> dot(const Array<T> &lhs, const Array<T> &rhs, af_mat_prop optLhs,
             af_mat_prop optRhs);

}  // namespace oneapi
}  // namespace arrayfire
