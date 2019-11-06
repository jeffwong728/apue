#include <limits.h>
#include <bzlib.h>
#include <zlib.h>
#include <lzma.h>
#include <GL/freeglut.h>
#include <jpeglib.h>
#include <turbojpeg.h>
#include <tiffio.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <szlib.h>
#ifdef __cplusplus
}
#endif
#include <Simd/SimdLib.h>
#include <tbb/tbb.h>
#include <pstl/algorithm>
#include <pstl/numeric>
#include <pstl/execution>
#include <pixman.h>
#include <pcre.h>
#include <pcrecpp.h>
#include <random>
#include <CL/cl2.hpp>
#include <sigc++/sigc++.h>
#include <png.h>
#include <ffi.h>
#include <iconv.h>
#include <dirent.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <gtest/gtest.h>
#include <glib.h>
#include <cairo.h>
#include <hdf5.h>
#include <H5Cpp.h>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/optional.hpp>
#include <opencv2/core.hpp>
#include "myalloc11.hpp"

namespace boost {
    namespace optional_config {
        template <> struct optional_uses_direct_storage_for<cv::Point2d> : boost::true_type {};
        template <> struct optional_uses_direct_storage_for<cv::Rect2d> : boost::true_type {};
    }
}

namespace {
TEST(BZip2Test, Version)
{
    EXPECT_EQ(std::string(BZ2_bzlibVersion()), std::string("1.0.6, 6-Sept-2010"));
}

TEST(ZLIBTest, Version)
{
    EXPECT_EQ(std::string(z_zlibVersion()), std::string("1.2.11"));
}

TEST(LibLZMATest, Version)
{
    EXPECT_EQ(lzma_version_number(), 50020042u);
    EXPECT_EQ(std::string(lzma_version_string()), std::string("5.2.4"));
}

TEST(GLUTTest, Version)
{
    EXPECT_EQ(glutGetWindow(), 0);
}

TEST(JPEGTest, Version)
{
    EXPECT_EQ(jpeg_quality_scaling(0), 5000);
}

TEST(TurboJPEGTest, Version)
{
    EXPECT_EQ(std::string(tjGetErrorStr()), std::string("No error"));
}

TEST(TIFFTest, Version)
{
    EXPECT_EQ(std::string(TIFFGetVersion()), std::string("LIBTIFF, Version 4.0.10\nCopyright (c) 1988-1996 Sam Leffler\nCopyright (c) 1991-1996 Silicon Graphics, Inc."));
}

TEST(SZIPTest, Version)
{
    EXPECT_EQ(SZ_encoder_enabled(), 1);
}

TEST(SIMDTest, Version)
{
    EXPECT_EQ(std::string(SimdVersion()), std::string("4.4.82"));
}

TEST(SIMDTest, CPU)
{
    EXPECT_EQ(SimdCpuInfo(SimdCpuInfoAvx2), 1);
    EXPECT_EQ(SimdCpuInfo(SimdCpuInfoAvx512f), 0);
    EXPECT_EQ(SimdCpuInfo(SimdCpuInfoAvx512bw), 0);
}

TEST(TBBTest, Timing)
{
    tbb::tick_count t1 = tbb::tick_count::now();
    tbb::tick_count t2 = tbb::tick_count::now();
    EXPECT_LT((t2-t1).seconds(), 1.);
}

TEST(PSTLTest, DotProduct)
{
    std::vector<int> v1{ 1, 2, 3 };
    std::vector<int> v2{ 4, 5, 6 };
    int res = std::transform_reduce(pstl::execution::par_unseq, v1.cbegin(), v1.cend(), v2.cbegin(), 0, std::plus<int>(), std::multiplies<int>());
    EXPECT_EQ(res, 32);
}

TEST(PixmanTest, Version)
{
    EXPECT_EQ(pixman_version(), 3800);
    EXPECT_EQ(std::string(pixman_version_string()), std::string("0.38.0"));
}

TEST(PCRETest, Version)
{
    EXPECT_EQ(std::string(pcre_version()), std::string("8.41 2017-07-05"));
}

TEST(OpenCLTest, Version)
{
    cl::Platform plat = cl::Platform::getDefault();
    cl::string ver;
    plat.getInfo(CL_PLATFORM_VERSION, &ver);
    EXPECT_EQ(ver, std::string("OpenCL 2.1 "));
}

TEST(SigCPPTest, Version)
{
    sigc::signal<int (const int&)> signal_print;
    signal_print.connect([](const int& v) { return v;});
    EXPECT_EQ(signal_print.emit(5), 5);

    sigc::slot_base slot_;
}

TEST(PNGTest, Version)
{
    EXPECT_EQ(png_access_version_number(), 10637);
}

TEST(FreeTypeTest, Version)
{
    FT_Library ftl;
    FT_Int amajor=0, aminor=0, apatch=0;
    FT_Init_FreeType(&ftl);
    FT_Library_Version(ftl, &amajor, &aminor, &apatch);
    EXPECT_EQ(amajor, 2);
    FT_Done_FreeType(ftl);
}

TEST(CairoTest, Version)
{
    EXPECT_EQ(cairo_version(), 11600);
}

TEST(HDF5Test, Version)
{
    unsigned majnum=0, minnum=0, relnum = 0;
    H5get_libversion(&majnum, &minnum, &relnum);
    EXPECT_EQ(majnum, 1);
    EXPECT_EQ(minnum, 10);
    EXPECT_EQ(relnum, 5);

    H5::H5Library::getLibVersion(majnum, minnum, relnum);
    EXPECT_EQ(majnum, 1);
    EXPECT_EQ(minnum, 10);
    EXPECT_EQ(relnum, 5);
}

TEST(BoostFileSystemTest, Basic)
{
    boost::filesystem::path pth("C:\\");
    EXPECT_EQ(boost::filesystem::is_directory(pth), true);
}

TEST(BoostLocaleTest, Basic)
{
    std::vector<std::string> allBackends = boost::locale::localization_backend_manager::global().get_all_backends();
    auto it = std::find(allBackends.cbegin(), allBackends.cend(), std::string("std"));
    EXPECT_NE(it, allBackends.cend());
}

TEST(BoostOptionalTest, Double)
{
    boost::optional<double> optval(10.0);
    EXPECT_EQ(*optval, 10.0);
}

TEST(BoostOptionalTest, CVPoint2d)
{
    boost::optional<cv::Point2d> optval;
    optval.emplace(10.0, 10.0);
    EXPECT_EQ(optval->x, 10.0);
    EXPECT_EQ(optval->y, 10.0);
}

TEST(BoostOptionalTest, CVRect2d)
{
    boost::optional<cv::Rect2d> optval;
    optval.emplace(10.0, 10.0, 20.0, 20.0);
    EXPECT_EQ(optval->x, 10.0);
    EXPECT_EQ(optval->y, 10.0);
}

TEST(MIMallocTest, Basic)
{
    std::vector<int, MyAlloc<int>> vec;
    vec.reserve(100);
    vec.push_back(1);
    vec.push_back(2);

    EXPECT_EQ(vec.capacity(), 100);
    EXPECT_EQ(vec.size(), 2);
}

}