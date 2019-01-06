#ifndef SPAM_UI_MISC_SPAM_UTILITY_H
#define SPAM_UI_MISC_SPAM_UTILITY_H
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#pragma warning( pop )

class SpamUtility : private boost::noncopyable
{
public:
    SpamUtility() = delete;

public:
    static cv::Mat GetMaskFromPath(const Geom::PathVector &pv, const cv::Size &sz);
};
#endif //SPAM_UI_MISC_SPAM_UTILITY_H