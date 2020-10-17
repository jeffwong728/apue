#include "helper.h"
#include <cstdlib>
#include <opencv2/highgui.hpp>

std::map<std::string, cv::Mat> UnitTestHelper::s_images_cache_;
std::map<std::string, std::tuple<cv::Mat, cv::Mat>> UnitTestHelper::s_gray_images_cache_;

void UnitTestHelper::ClearImagesCache()
{
    s_images_cache_.clear();
    s_gray_images_cache_.clear();
}

cv::Mat UnitTestHelper::GetImage(const boost::filesystem::path &imagePath)
{
    auto fIt = s_images_cache_.find(imagePath.generic_string());
    if (fIt != s_images_cache_.end())
    {
        return fIt->second;
    }
    else
    {
        cv::Mat img = cv::imread(cv::String(imagePath.string().c_str()), cv::IMREAD_UNCHANGED);
        s_images_cache_[imagePath.generic_string()] = img;

        return img;
    }
}

void UnitTestHelper::WriteImage(const cv::Mat &img, const boost::filesystem::path &imagePath)
{
    cv::imwrite(cv::String(imagePath.string().c_str()), img);
}

std::tuple<cv::Mat, cv::Mat> UnitTestHelper::GetGrayScaleImage(const boost::filesystem::path &imagePath)
{
    auto fIt = s_gray_images_cache_.find(imagePath.generic_string());
    if (fIt != s_gray_images_cache_.end())
    {
        return fIt->second;
    }
    else
    {
        cv::Mat img = cv::imread(cv::String(imagePath.string().c_str()), cv::IMREAD_UNCHANGED);

        int dph = img.depth();
        int cnl = img.channels();

        cv::Mat colorImg;
        cv::Mat grayImg;
        if (CV_8U == dph && (1 == cnl || 3 == cnl || 4 == cnl))
        {
            if (1 == cnl)
            {
                grayImg = img;
                cv::cvtColor(img, colorImg, cv::COLOR_GRAY2BGRA);
            }
            else if (3 == cnl)
            {
                cv::cvtColor(img, colorImg, cv::COLOR_BGR2BGRA);
                cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);
            }
            else
            {
                colorImg = img;
                cv::cvtColor(img, grayImg, cv::COLOR_BGRA2GRAY);
            }
        }

        s_gray_images_cache_[imagePath.generic_string()] = std::make_tuple(grayImg, colorImg);
        return { grayImg , colorImg };
    }
}

boost::filesystem::path UnitTestHelper::GetFullPath(const boost::filesystem::path &rPath)
{
    boost::filesystem::path utRootDir(std::string(std::getenv("SPAM_ROOT_DIR")));
    utRootDir.append(std::string("test"));
    utRootDir.append(std::string("data"));
    utRootDir.append(std::string("images"));
    utRootDir.append(rPath.string());

    return utRootDir;
}
