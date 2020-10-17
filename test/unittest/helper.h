#ifndef SPAM_UNITTEST_HELPER_H
#define SPAM_UNITTEST_HELPER_H
#include <tuple>
#include <array>
#include <map>
#include <cassert>
#include <boost/filesystem.hpp>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <boost/core/noncopyable.hpp>

class UnitTestHelper : private boost::noncopyable
{
public:
    UnitTestHelper() = delete;
    using Color = std::array<uint8_t, 4>;

public:
    static void ClearImagesCache();
    static cv::Mat GetImage(const boost::filesystem::path &imagePath);
    static void WriteImage(const cv::Mat &img, const boost::filesystem::path &imagePath);
    static std::tuple<cv::Mat, cv::Mat> GetGrayScaleImage(const boost::filesystem::path &rPath);
    static boost::filesystem::path GetFullPath(const boost::filesystem::path &rPath);

private:
    static std::map<std::string, cv::Mat> s_images_cache_;
    static std::map<std::string, std::tuple<cv::Mat, cv::Mat>> s_gray_images_cache_;
};

#endif //SPAM_UNITTEST_HELPER_H