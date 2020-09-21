#include "precomp.hpp"
#include "dict_impl.hpp"
#include <region/region_impl.hpp>
#include <hdf5/h5group_impl.hpp>
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

cv::Ptr<Dict> Dict::GenEmpty()
{
    return makePtr<DictImpl>();
}

int DictImpl::SetInt(const cv::String &name, const int val)
{
    ints_[name] = val;
    return MLR_SUCCESS;
}

int DictImpl::SetReal64(const cv::String &name, const double val)
{
    reals_[name] = val;
    return MLR_SUCCESS;
}

int DictImpl::SetReal32(const cv::String &name, const float val)
{
    singles_[name] = val;
    return MLR_SUCCESS;
}

int DictImpl::SetString(const cv::String &name, const cv::String &val)
{
    strs_[name] = val;
    return MLR_SUCCESS;
}

int DictImpl::SetMat(const cv::String &name, const cv::Mat &val)
{
    mats_[name] = val;
    return MLR_SUCCESS;
}

int DictImpl::SetRegion(const cv::String &name, const cv::Ptr<Region> &val)
{
    rgns_[name] = val;
    return MLR_SUCCESS;
}

int DictImpl::SetScalar(const cv::String &name, const cv::Scalar &val)
{
    scalars_[name] = val;
    return MLR_SUCCESS;
}

int DictImpl::SetH5Group(const cv::String &name, const cv::Ptr<H5Group> &val)
{
    groups_[name] = val;
    return MLR_SUCCESS;
}

bool DictImpl::Empty() const
{
    return ints_.empty() && reals_.empty() && strs_.empty() && mats_.empty() && singles_.empty() && scalars_.empty();
}

int DictImpl::GetInt(const cv::String &name, const int defaultVal) const
{
    auto fIt = ints_.find(name);
    if (fIt != ints_.cend())
    {
        return fIt->second;
    }
    else
    {
        return defaultVal;
    }
}

double DictImpl::GetReal64(const cv::String &name, const double defaultVal) const
{
    auto fIt = reals_.find(name);
    if (fIt != reals_.cend())
    {
        return fIt->second;
    }
    else
    {
        return defaultVal;
    }
}

float DictImpl::GetReal32(const cv::String &name, const float defaultVal) const
{
    auto fIt = singles_.find(name);
    if (fIt != singles_.cend())
    {
        return fIt->second;
    }
    else
    {
        return defaultVal;
    }
}

cv::String DictImpl::GetString(const cv::String &name) const
{
    auto fIt = strs_.find(name);
    if (fIt != strs_.cend())
    {
        return fIt->second;
    }
    else
    {
        return cv::String();
    }
}

cv::Mat DictImpl::GetMat(const cv::String &name) const
{
    auto fIt = mats_.find(name);
    if (fIt != mats_.cend())
    {
        return fIt->second;
    }
    else
    {
        return cv::Mat();
    }
}

cv::Ptr<Region> DictImpl::GetRegion(const cv::String &name) const
{
    auto fIt = rgns_.find(name);
    if (fIt != rgns_.cend())
    {
        return fIt->second;
    }
    else
    {
        return cv::makePtr<RegionImpl>();
    }
}

cv::Scalar DictImpl::GetScalar(const cv::String &name, const cv::Scalar &defaultVal) const
{
    auto fIt = scalars_.find(name);
    if (fIt != scalars_.cend())
    {
        return fIt->second;
    }
    else
    {
        return defaultVal;
    }
}

cv::Ptr<H5Group> DictImpl::GetH5Group(const cv::String &name) const
{
    auto fIt = groups_.find(name);
    if (fIt != groups_.cend())
    {
        return fIt->second;
    }
    else
    {
        return cv::makePtr<H5GroupImpl>();
    }
}

}
}
