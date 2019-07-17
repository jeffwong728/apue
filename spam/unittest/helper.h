#ifndef SPAM_UNITTEST_HELPER_H
#define SPAM_UNITTEST_HELPER_H
#include <tuple>
#include <array>
#include <cassert>
#include <experimental/filesystem>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <boost/core/noncopyable.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4244 4267)
#include <2geom/2geom.h>
#include <2geom/cairo-path-sink.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#pragma warning( pop )

#ifdef _DEBUG
constexpr bool vtune_build = false;
#else
constexpr bool vtune_build = true;
#endif

class UnitTestHelper : private boost::noncopyable
{
public:
    UnitTestHelper() = delete;
    using Color = std::array<uint8_t, 4>;

public:
    static cv::Mat GetImage(const std::experimental::filesystem::path &rPath);
    static void WriteImage(const cv::Mat &img, const std::experimental::filesystem::path &rPath);
    static std::tuple<cv::Mat, cv::Mat> GetGrayScaleImage(const std::experimental::filesystem::path &rPath);
    static void DrawPathToImage(const Geom::PathVector &pth, const Color& color, cv::Mat &img);
};

#endif //SPAM_UNITTEST_HELPER_H