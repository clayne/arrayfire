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
#include <iostream>
#include <string>
#include <vector>

using af::array;
using af::cdouble;
using af::cfloat;
using af::dim4;
using std::endl;
using std::string;
using std::vector;

template<typename T>
class ImageIO : public ::testing::Test {
   public:
    virtual void SetUp() {}
};

typedef ::testing::Types<float> TestTypes;

// register the type list
TYPED_TEST_SUITE(ImageIO, TestTypes);

void loadImageTest(string pTestFile, string pImageFile, const bool isColor) {
    IMAGEIO_ENABLED_CHECK();

    vector<dim4> numDims;

    vector<vector<float>> in;
    vector<vector<float>> tests;
    readTests<float, float, float>(pTestFile, numDims, in, tests);
    dim4 dims = numDims[0];

    af_array imgArray = 0;
    ASSERT_SUCCESS(af_load_image(&imgArray, pImageFile.c_str(), isColor));

    // Get result
    float* imgData = new float[dims.elements()];
    ASSERT_SUCCESS(af_get_data_ptr((void*)imgData, imgArray));

    bool isJPEG = false;
    if (pImageFile.find(".jpg") != string::npos) { isJPEG = true; }

    // Compare result
    size_t nElems = in[0].size();
    for (size_t elIter = 0; elIter < nElems; ++elIter) {
        if (isJPEG)  // Allow +- 1 because of compression when testing JPG
            ASSERT_NEAR(in[0][elIter], imgData[elIter], 1)
                << "at: " << elIter << endl;
        else
            ASSERT_EQ(in[0][elIter], imgData[elIter])
                << "at: " << elIter << endl;
    }

    // Delete
    delete[] imgData;

    if (imgArray != 0) af_release_array(imgArray);
}

TYPED_TEST(ImageIO, ColorSmall) {
    loadImageTest(string(TEST_DIR "/imageio/color_small.test"),
                  string(TEST_DIR "/imageio/color_small.png"), true);
}

TYPED_TEST(ImageIO, GraySmall) {
    loadImageTest(string(TEST_DIR "/imageio/gray_small.test"),
                  string(TEST_DIR "/imageio/gray_small.jpg"), false);
}

TYPED_TEST(ImageIO, GraySeq) {
    loadImageTest(string(TEST_DIR "/imageio/gray_seq.test"),
                  string(TEST_DIR "/imageio/gray_seq.png"), false);
}

TYPED_TEST(ImageIO, ColorSeq) {
    loadImageTest(string(TEST_DIR "/imageio/color_seq.test"),
                  string(TEST_DIR "/imageio/color_seq.png"), true);
}

void loadimageArgsTest(string pImageFile, const bool isColor, af_err err) {
    IMAGEIO_ENABLED_CHECK();

    af_array imgArray = 0;

    ASSERT_EQ(err, af_load_image(&imgArray, pImageFile.c_str(), isColor));

    if (imgArray != 0) af_release_array(imgArray);
}

TYPED_TEST(ImageIO, InvalidArgsMissingFile) {
    loadimageArgsTest(string(TEST_DIR "/imageio/nofile.png"), false,
                      AF_ERR_RUNTIME);
}

TYPED_TEST(ImageIO, InvalidArgsWrongExt) {
    loadimageArgsTest(string(TEST_DIR "/imageio/image.wrongext"), true,
                      AF_ERR_NOT_SUPPORTED);
}

////////////////////////////////// CPP //////////////////////////////////////

using af::anyTrue;
using af::deleteImageMem;
using af::loadImage;
using af::loadImageMem;
using af::saveImageMem;
using af::span;

TEST(ImageIO, CPP) {
    IMAGEIO_ENABLED_CHECK();

    vector<dim4> numDims;

    vector<vector<float>> in;
    vector<vector<float>> tests;
    readTests<float, float, float>(string(TEST_DIR "/imageio/color_small.test"),
                                   numDims, in, tests);

    dim4 dims = numDims[0];
    array img =
        loadImage(string(TEST_DIR "/imageio/color_small.png").c_str(), true);

    // Get result
    float* imgData = new float[dims.elements()];
    img.host((void*)imgData);

    // Compare result
    size_t nElems = in[0].size();
    for (size_t elIter = 0; elIter < nElems; ++elIter) {
        ASSERT_EQ(in[0][elIter], imgData[elIter]) << "at: " << elIter << endl;
    }

    // Delete
    delete[] imgData;
}

TEST(ImageIO, SavePNGCPP) {
    IMAGEIO_ENABLED_CHECK();

    array input(10, 10, 3, f32);

    input(span, span, span) = 0;
    input(0, 0, 0)          = 255;
    input(0, 9, 1)          = 255;
    input(9, 0, 2)          = 255;
    input(9, 9, span)       = 255;

    std::string testname  = getTestName() + "_" + getBackendName(true);
    std::string imagename = "SaveCPP_" + testname + ".png";

    saveImage(imagename.c_str(), input);
    array out = loadImage(imagename.c_str(), true);

    ASSERT_FALSE(anyTrue<bool>(out - input));
}

TEST(ImageIO, SaveBMPCPP) {
    IMAGEIO_ENABLED_CHECK();

    array input(10, 10, 3, f32);

    input(span, span, span) = 0;
    input(0, 0, 0)          = 255;
    input(0, 9, 1)          = 255;
    input(9, 0, 2)          = 255;
    input(9, 9, span)       = 255;

    std::string testname  = getTestName() + "_" + getBackendName(true);
    std::string imagename = "SaveCPP_" + testname + ".bmp";

    saveImage(imagename.c_str(), input);
    array out = loadImage(imagename.c_str(), true);

    ASSERT_FALSE(anyTrue<bool>(out - input));
}

TEST(ImageMem, SaveMemPNG) {
    IMAGEIO_ENABLED_CHECK();

    array img =
        loadImage(string(TEST_DIR "/imageio/color_seq.png").c_str(), true);

    void* savedMem = saveImageMem(img, AF_FIF_PNG);

    array loadMem = loadImageMem(savedMem);

    ASSERT_FALSE(anyTrue<bool>(img - loadMem));

    deleteImageMem(savedMem);
}

TEST(ImageMem, SaveMemJPG1) {
    IMAGEIO_ENABLED_CHECK();

    array img =
        loadImage(string(TEST_DIR "/imageio/color_seq.png").c_str(), false);
    saveImage("color_seq1.jpg", img);

    void* savedMem = saveImageMem(img, AF_FIF_JPEG);

    array loadMem = loadImageMem(savedMem);
    array imgJPG  = loadImage("color_seq1.jpg", false);

    ASSERT_FALSE(anyTrue<bool>(imgJPG - loadMem));

    deleteImageMem(savedMem);
}

TEST(ImageMem, SaveMemJPG3) {
    IMAGEIO_ENABLED_CHECK();

    array img =
        loadImage(string(TEST_DIR "/imageio/color_seq.png").c_str(), true);
    saveImage("color_seq3.jpg", img);

    void* savedMem = saveImageMem(img, AF_FIF_JPEG);

    array loadMem = loadImageMem(savedMem);
    array imgJPG  = loadImage("color_seq3.jpg", true);

    ASSERT_FALSE(anyTrue<bool>(imgJPG - loadMem));

    deleteImageMem(savedMem);
}

TEST(ImageMem, SaveMemBMP) {
    IMAGEIO_ENABLED_CHECK();

    array img =
        loadImage(string(TEST_DIR "/imageio/color_rand.png").c_str(), true);

    void* savedMem = saveImageMem(img, AF_FIF_BMP);

    array loadMem = loadImageMem(savedMem);

    ASSERT_FALSE(anyTrue<bool>(img - loadMem));

    deleteImageMem(savedMem);
}

TEST(ImageIO, LoadImage16CPP) {
    IMAGEIO_ENABLED_CHECK();

    vector<dim4> numDims;

    vector<vector<float>> in;
    vector<vector<float>> tests;
    readTests<float, float, float>(
        string(TEST_DIR "/imageio/color_seq_16.test"), numDims, in, tests);

    dim4 dims = numDims[0];

    array img =
        loadImage(string(TEST_DIR "/imageio/color_seq_16.png").c_str(), true);
    ASSERT_EQ(img.type(), f32);  // loadImage should always return float

    // Get result
    float* imgData = new float[dims.elements()];
    img.host((void*)imgData);

    // Compare result
    size_t nElems = in[0].size();
    for (size_t elIter = 0; elIter < nElems; ++elIter) {
        ASSERT_EQ(in[0][elIter], imgData[elIter]) << "at: " << elIter << endl;
    }

    // Delete
    delete[] imgData;
}

TEST(ImageIO, SaveImage16CPP) {
    IMAGEIO_ENABLED_CHECK();

    dim4 dims(16, 24, 3);

    array input     = randu(dims, u16);
    array input_255 = floor(input.as(f32) / 257);

    std::string testname  = getTestName() + "_" + getBackendName(true);
    std::string imagename = "saveImage16CPP_" + testname + ".png";

    saveImage(imagename.c_str(), input);

    array img = loadImage(imagename.c_str(), true);

    ASSERT_EQ(img.type(), f32);  // loadImage should always return float
    ASSERT_IMAGES_NEAR(input_255, img, 0.001);
}

////////////////////////////////////////////////////////////////////////////////
// Image IO Native Tests
////////////////////////////////////////////////////////////////////////////////

using af::dtype_traits;
using af::loadImageNative;
using af::saveImageNative;

template<typename T>
void loadImageNativeCPPTest(string pTestFile, string pImageFile) {
    IMAGEIO_ENABLED_CHECK();

    vector<dim4> numDims;

    vector<vector<float>> in;
    vector<vector<float>> tests;
    readTests<float, float, float>(pTestFile, numDims, in, tests);

    dim4 dims = numDims[0];
    array img = loadImageNative(pImageFile.c_str());
    ASSERT_EQ(img.type(), (af_dtype)dtype_traits<T>::af_type);

    // Get result
    T* imgData = new T[dims.elements()];
    img.host((void*)imgData);

    // Compare result
    size_t nElems = in[0].size();
    for (size_t elIter = 0; elIter < nElems; ++elIter) {
        ASSERT_EQ(in[0][elIter], imgData[elIter]) << "at: " << elIter << endl;
    }

    // Delete
    delete[] imgData;
}

TEST(ImageIONative, LoadImageNative8CPP) {
    loadImageNativeCPPTest<uchar>(string(TEST_DIR "/imageio/color_small.test"),
                                  string(TEST_DIR "/imageio/color_small.png"));
}

TEST(ImageIONative, LoadImageNative16SmallCPP) {
    loadImageNativeCPPTest<ushort>(
        string(TEST_DIR "/imageio/color_small_16.test"),
        string(TEST_DIR "/imageio/color_small_16.png"));
}

TEST(ImageIONative, LoadImageNative16ColorCPP) {
    loadImageNativeCPPTest<ushort>(
        string(TEST_DIR "/imageio/color_seq_16.test"),
        string(TEST_DIR "/imageio/color_seq_16.png"));
}

TEST(ImageIONative, LoadImageNative16GrayCPP) {
    loadImageNativeCPPTest<ushort>(string(TEST_DIR "/imageio/gray_seq_16.test"),
                                   string(TEST_DIR "/imageio/gray_seq_16.png"));
}

template<typename T>
void saveLoadImageNativeCPPTest(dim4 dims) {
    IMAGEIO_ENABLED_CHECK();

    array input = randu(dims, (af_dtype)dtype_traits<T>::af_type);

    std::string imagename = getTestName() + "_" + getBackendName(true) + ".png";

    saveImageNative(imagename.c_str(), input);

    array loaded = loadImageNative(imagename.c_str());
    ASSERT_EQ(loaded.type(), input.type());

    ASSERT_FALSE(anyTrue<bool>(input - loaded));
}

TEST(ImageIONative, SaveLoadImageNative8CPP) {
    saveLoadImageNativeCPPTest<uchar>(dim4(480, 720, 3, 1));
}

TEST(ImageIONative, SaveLoadImageNative16SmallCPP) {
    saveLoadImageNativeCPPTest<ushort>(dim4(8, 12, 3, 1));
}

TEST(ImageIONative, SaveLoadImageNative16ColorCPP) {
    saveLoadImageNativeCPPTest<ushort>(dim4(480, 720, 3, 1));
}

TEST(ImageIONative, SaveLoadImageNative16GrayCPP) {
    saveLoadImageNativeCPPTest<ushort>(dim4(24, 32, 1, 1));
}
