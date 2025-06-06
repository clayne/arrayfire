/*******************************************************
 * Copyright (c) 2014, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 ********************************************************/

#include <arrayfire.h>
#include <gtest/gtest.h>
#include <testHelpers.hpp>
#include <af/dim4.hpp>
#include <af/traits.hpp>
#include <string>
#include <vector>

using af::cdouble;
using af::cfloat;
using af::dim4;
using af::dtype_traits;
using std::endl;
using std::string;
using std::vector;

template<typename T>
class Diff1 : public ::testing::Test {
   public:
    virtual void SetUp() {
        subMat0.push_back(af_make_seq(1, 4, 1));
        subMat0.push_back(af_make_seq(0, 2, 1));
        subMat0.push_back(af_make_seq(0, 1, 1));

        subMat1.push_back(af_make_seq(0, 4, 1));
        subMat1.push_back(af_make_seq(1, 3, 1));
        subMat1.push_back(af_make_seq(1, 3, 1));

        subMat2.push_back(af_make_seq(1, 5, 1));
        subMat2.push_back(af_make_seq(0, 3, 1));
        subMat2.push_back(af_make_seq(0, 2, 1));
    }
    vector<af_seq> subMat0;
    vector<af_seq> subMat1;
    vector<af_seq> subMat2;
};

// create a list of types to be tested
typedef ::testing::Types<float, cfloat, double, cdouble, int, unsigned, intl,
                         uintl, char, signed char, unsigned char, short, ushort>
    TestTypes;

// register the type list
TYPED_TEST_SUITE(Diff1, TestTypes);

template<typename T>
void diff1Test(string pTestFile, unsigned dim, bool isSubRef = false,
               const vector<af_seq> *seqv = NULL) {
    SUPPORTED_TYPE_CHECK(T);

    vector<dim4> numDims;

    vector<vector<T>> in;
    vector<vector<T>> tests;
    readTests<T, T, int>(pTestFile, numDims, in, tests);
    dim4 dims = numDims[0];

    af_array inArray   = 0;
    af_array outArray  = 0;
    af_array tempArray = 0;
    // Get input array
    if (isSubRef) {
        ASSERT_SUCCESS(af_create_array(&tempArray, &(in[0].front()),
                                       dims.ndims(), dims.get(),
                                       (af_dtype)dtype_traits<T>::af_type));

        ASSERT_SUCCESS(
            af_index(&inArray, tempArray, seqv->size(), &seqv->front()));
    } else {
        ASSERT_SUCCESS(af_create_array(&inArray, &(in[0].front()), dims.ndims(),
                                       dims.get(),
                                       (af_dtype)dtype_traits<T>::af_type));
    }

    // Run diff1
    ASSERT_SUCCESS(af_diff1(&outArray, inArray, dim));

    // Compare result
    for (size_t testIter = 0; testIter < tests.size(); ++testIter) {
        vector<T> currGoldBar = tests[testIter];
        dim4 goldDims;
        ASSERT_SUCCESS(af_get_dims(&goldDims[0], &goldDims[1], &goldDims[2],
                                   &goldDims[3], inArray));
        goldDims[dim]--;

        ASSERT_VEC_ARRAY_EQ(currGoldBar, goldDims, outArray);
    }

    if (inArray != 0) af_release_array(inArray);
    if (outArray != 0) af_release_array(outArray);
    if (tempArray != 0) af_release_array(tempArray);
}

TYPED_TEST(Diff1, Vector0) {
    diff1Test<TypeParam>(string(TEST_DIR "/diff1/vector0.test"), 0);
}

TYPED_TEST(Diff1, Matrix0) {
    diff1Test<TypeParam>(string(TEST_DIR "/diff1/matrix0.test"), 0);
}

TYPED_TEST(Diff1, Matrix1) {
    diff1Test<TypeParam>(string(TEST_DIR "/diff1/matrix1.test"), 1);
}

// Diff on 0 dimension
TYPED_TEST(Diff1, Basic0) {
    diff1Test<TypeParam>(string(TEST_DIR "/diff1/basic0.test"), 0);
}

// Diff on 1 dimension
TYPED_TEST(Diff1, Basic1) {
    diff1Test<TypeParam>(string(TEST_DIR "/diff1/basic1.test"), 1);
}

// Diff on 2 dimension
TYPED_TEST(Diff1, Basic2) {
    diff1Test<TypeParam>(string(TEST_DIR "/diff1/basic2.test"), 2);
}

// Diff on 0 dimension subref
TYPED_TEST(Diff1, Subref0) {
    diff1Test<TypeParam>(string(TEST_DIR "/diff1/subref0.test"), 0, true,
                         &(this->subMat0));
}

// Diff on 1 dimension subref
TYPED_TEST(Diff1, Subref1) {
    diff1Test<TypeParam>(string(TEST_DIR "/diff1/subref1.test"), 1, true,
                         &(this->subMat1));
}

// Diff on 2 dimension subref
TYPED_TEST(Diff1, Subref2) {
    diff1Test<TypeParam>(string(TEST_DIR "/diff1/subref2.test"), 2, true,
                         &(this->subMat2));
}

template<typename T>
void diff1ArgsTest(string pTestFile) {
    SUPPORTED_TYPE_CHECK(T);

    vector<dim4> numDims;

    vector<vector<T>> in;
    vector<vector<T>> tests;
    readTests<T, T, int>(pTestFile, numDims, in, tests);
    dim4 dims = numDims[0];

    af_array inArray  = 0;
    af_array outArray = 0;

    ASSERT_SUCCESS(af_create_array(&inArray, &(in[0].front()), dims.ndims(),
                                   dims.get(),
                                   (af_dtype)dtype_traits<T>::af_type));

    ASSERT_EQ(AF_ERR_ARG, af_diff1(&outArray, inArray, -1));
    ASSERT_EQ(AF_ERR_ARG, af_diff1(&outArray, inArray, 5));

    if (inArray != 0) af_release_array(inArray);
    if (outArray != 0) af_release_array(outArray);
}

TYPED_TEST(Diff1, InvalidArgs) {
    diff1ArgsTest<TypeParam>(string(TEST_DIR "/diff1/basic0.test"));
}

////////////////////////////////////// CPP ////////////////////////////////////
//

using af::array;
using af::constant;
using af::deviceGC;
using af::diff1;
using af::sum;

TEST(Diff1, DiffLargeDim) {
    const size_t largeDim = 65535 * 32 + 1;

    deviceGC();
    {
        array in   = constant(1, largeDim);
        array diff = diff1(in, 0);
        float s    = sum<float>(diff, 1);
        ASSERT_EQ(s, 0.f);

        in   = constant(1, 1, largeDim);
        diff = diff1(in, 1);
        s    = sum<float>(diff, 1);
        ASSERT_EQ(s, 0.f);

        in   = constant(1, 1, 1, largeDim);
        diff = diff1(in, 2);
        s    = sum<float>(diff, 1);
        ASSERT_EQ(s, 0.f);

        in   = constant(1, 1, 1, 1, largeDim);
        diff = diff1(in, 3);
        s    = sum<float>(diff, 1);
        ASSERT_EQ(s, 0.f);
    }
}

TEST(Diff1, CPP) {
    const unsigned dim = 0;
    vector<dim4> numDims;

    vector<vector<float>> in;
    vector<vector<float>> tests;
    readTests<float, float, int>(string(TEST_DIR "/diff1/matrix0.test"),
                                 numDims, in, tests);
    dim4 dims = numDims[0];

    array input(dims, &(in[0].front()));
    array output = diff1(input, dim);

    // Compare result
    for (size_t testIter = 0; testIter < tests.size(); ++testIter) {
        vector<float> currGoldBar = tests[testIter];
        dim4 goldDims             = dims;
        goldDims[dim]--;

        ASSERT_VEC_ARRAY_EQ(currGoldBar, goldDims, output);
    }
}
