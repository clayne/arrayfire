/*******************************************************
 * Copyright (c) 2022, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 ********************************************************/

#include <Array.hpp>
#include <common/half.hpp>
#include <diagonal.hpp>
#include <err_oneapi.hpp>
#include <kernel/diagonal.hpp>
#include <math.hpp>
#include <af/dim4.hpp>

using arrayfire::common::half;

namespace arrayfire {
namespace oneapi {
template<typename T>
Array<T> diagCreate(const Array<T> &in, const int num) {
    int size     = in.dims()[0] + std::abs(num);
    int batch    = in.dims()[1];
    Array<T> out = createEmptyArray<T>(dim4(size, size, batch));

    kernel::diagCreate<T>(out, in, num);

    return out;
}

template<typename T>
Array<T> diagExtract(const Array<T> &in, const int num) {
    const dim_t *idims = in.dims().get();
    dim_t size         = std::min(idims[0], idims[1]) - std::abs(num);
    Array<T> out       = createEmptyArray<T>(dim4(size, 1, idims[2], idims[3]));

    kernel::diagExtract<T>(out, in, num);

    return out;
}

#define INSTANTIATE_DIAGONAL(T)                                          \
    template Array<T> diagExtract<T>(const Array<T> &in, const int num); \
    template Array<T> diagCreate<T>(const Array<T> &in, const int num);

INSTANTIATE_DIAGONAL(float)
INSTANTIATE_DIAGONAL(double)
INSTANTIATE_DIAGONAL(cfloat)
INSTANTIATE_DIAGONAL(cdouble)
INSTANTIATE_DIAGONAL(int)
INSTANTIATE_DIAGONAL(uint)
INSTANTIATE_DIAGONAL(intl)
INSTANTIATE_DIAGONAL(uintl)
INSTANTIATE_DIAGONAL(char)
INSTANTIATE_DIAGONAL(schar)
INSTANTIATE_DIAGONAL(uchar)
INSTANTIATE_DIAGONAL(short)
INSTANTIATE_DIAGONAL(ushort)
INSTANTIATE_DIAGONAL(half)

}  // namespace oneapi
}  // namespace arrayfire
