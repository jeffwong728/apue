/*M///////////////////////////////////////////////////////////////////////////////////////
 //
 //  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 //
 //  By downloading, copying, installing or using the software you agree to this license.
 //  If you do not agree to this license, do not download, install,
 //  copy or use the software.
 //
 //
 //                           License Agreement
 //                For Open Source Computer Vision Library
 //
 // Copyright (C) 2014, OpenCV Foundation, all rights reserved.
 // Third party copyrights are property of their respective owners.
 //
 // Redistribution and use in source and binary forms, with or without modification,
 // are permitted provided that the following conditions are met:
 //
 //   * Redistribution's of source code must retain the above copyright notice,
 //     this list of conditions and the following disclaimer.
 //
 //   * Redistribution's in binary form must reproduce the above copyright notice,
 //     this list of conditions and the following disclaimer in the documentation
 //     and/or other materials provided with the distribution.
 //
 //   * The name of the copyright holders may not be used to endorse or promote products
 //     derived from this software without specific prior written permission.
 //
 // This software is provided by the copyright holders and contributors "as is" and
 // any express or implied warranties, including, but not limited to, the implied
 // warranties of merchantability and fitness for a particular purpose are disclaimed.
 // In no event shall the Intel Corporation or contributors be liable for any direct,
 // indirect, incidental, special, exemplary, or consequential damages
 // (including, but not limited to, procurement of substitute goods or services;
 // loss of use, data, or profits; or business interruption) however caused
 // and on any theory of liability, whether in contract, strict liability,
 // or tort (including negligence or otherwise) arising in any way out of
 // the use of this software, even if advised of the possibility of such damage.
 //
 //M*/

#ifndef __OPENCV_MVLAB_PRECOMP__
#define __OPENCV_MVLAB_PRECOMP__

#include "myalloc11.hpp"
#include "uvector.h"
#include <set>
#include <map>
#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <stack>
#include <numeric>
#include <filesystem>
#include <fstream>
#include <variant>
#include <H5Cpp.h>
#pragma warning( push )
#pragma warning( disable : 4389 4310 )
#include <vectorclass.h>
#pragma warning( pop )
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgcodecs.hpp>
#include <tbb/tbb.h>
#include <tbb/parallel_invoke.h>
#include <tbb/scalable_allocator.h>
#include <cairomm/cairomm.h>
#include <boost/optional.hpp>
#include <boost/variant2/variant.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/export.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#pragma warning( push )
#pragma warning( disable : 4100 4702)
#include <boost/geometry.hpp>
#include <boost/geometry/core/exception.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
#pragma warning( pop )
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267 4244 )
#include <2geom/2geom.h>
#include <2geom/circle.h>
#include <2geom/ellipse.h>
#include <2geom/path-sink.h>
#include <2geom/path-intersection.h>
#include <2geom/cairo-path-sink.h>
#pragma warning( pop )

constexpr int XY_SHIFT = 10;
constexpr int XY_DELTA = 1 << XY_SHIFT >> 1;
constexpr int XY_ONE = 1 << XY_SHIFT;
constexpr float F_XY_ONE = 1 << XY_SHIFT;
constexpr double D_XY_ONE = 1 << XY_SHIFT;
constexpr int K_NO = 0;
constexpr int K_YES = 1;
constexpr int K_UNKNOWN = 2;
constexpr float  G_F_TOL = 0.0001f;
constexpr double G_D_TOL = 0.0001;

template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Point> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Point2f> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Point2d> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Point3f> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Point3d> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Rect> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Rect2f> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Rect2d> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Scalar> : boost::true_type {};

using Point2fSequence                   = std::vector<cv::Point2f>;
using ScalableIntSequence               = std::vector<int, MyAlloc<int>>;
using ScalableIntSequenceSequence       = std::vector<ScalableIntSequence, MyAlloc<ScalableIntSequence>>;
using ScalableFloatSequence             = std::vector<float, MyAlloc<float>>;
using ScalableDoubleSequence            = std::vector<double, MyAlloc<double>>;
using ScalablePointSequence             = std::vector<cv::Point, MyAlloc<cv::Point>>;
using ScalablePoint2fSequence           = std::vector<cv::Point2f, MyAlloc<cv::Point2f>>;
using ScalablePoint2dSequence           = std::vector<cv::Point2d, MyAlloc<cv::Point2d>>;
using ScalableRectSequence              = std::vector<cv::Rect, MyAlloc<cv::Rect>>;
using ScalablePoint2fSequenceSequence   = std::vector<ScalablePoint2fSequence, MyAlloc<ScalablePoint2fSequence>>;
using ScalableSize2fSequence            = std::vector<cv::Size2f, MyAlloc<cv::Size2f>>;
using UScalableBoolSequence             = ao::uvector<bool, MyAlloc<bool>>;
using UScalableIntSequence              = ao::uvector<int, MyAlloc<int>>;
using UScalableFloatSequence            = ao::uvector<float, MyAlloc<float>>;
using UScalableDoubleSequence           = ao::uvector<double, MyAlloc<double>>;
using UScalableUCharSequence            = ao::uvector<uint8_t, MyAlloc<uint8_t>>;
using UScalablePointSequence            = ao::uvector<cv::Point, MyAlloc<cv::Point>>;
using UScalablePoint2fSequence          = ao::uvector<cv::Point2f, MyAlloc<cv::Point2f>>;
using UScalablePoint2dSequence          = ao::uvector<cv::Point2d, MyAlloc<cv::Point2d>>;
using UScalableRectSequence             = ao::uvector<cv::Rect, MyAlloc<cv::Rect>>;

BOOST_GEOMETRY_REGISTER_POINT_2D(cv::Point2f, float, boost::geometry::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_RING(ScalablePoint2fSequence)

namespace boost {
namespace serialization {

template<class Archive, class T>
void serialize(Archive & ar, cv::Point_<T> &v, const unsigned int version)
{
    ar & boost::serialization::make_nvp("x", v.x);
    ar & boost::serialization::make_nvp("y", v.y);
}

template<class Archive, class T>
void serialize(Archive & ar, cv::Point3_<T> &v, const unsigned int version)
{
    ar & boost::serialization::make_nvp("x", v.x);
    ar & boost::serialization::make_nvp("y", v.y);
    ar & boost::serialization::make_nvp("z", v.z);
}

template<class Archive, class T>
void serialize(Archive & ar, cv::Size_<T> &v, const unsigned int version)
{
    ar & boost::serialization::make_nvp("width", v.width);
    ar & boost::serialization::make_nvp("height", v.height);
}

template<class Archive, class T>
void serialize(Archive & ar, cv::Rect_<T> &v, const unsigned int version)
{
    ar & boost::serialization::make_nvp("x", v.x);
    ar & boost::serialization::make_nvp("y", v.y);
    ar & boost::serialization::make_nvp("width", v.width);
    ar & boost::serialization::make_nvp("height", v.height);
}

template<class Archive, class T>
void serialize(Archive & ar, cv::Scalar_<T> &v, const unsigned int version)
{
    ar & boost::serialization::make_nvp("v0", v[0]);
    ar & boost::serialization::make_nvp("v1", v[1]);
    ar & boost::serialization::make_nvp("v2", v[2]);
    ar & boost::serialization::make_nvp("v3", v[3]);
}

template<class Archive, class T>
void serialize(Archive & ar, cv::Ptr<T> &v, const unsigned int version)
{
    ar & boost::serialization::make_nvp("CVPtr", (std::shared_ptr<T>&)(v));
}

}
}

typedef boost::mpl::vector<bool, char, short, int, long, long long, float, double> OptTypes0;
typedef boost::mpl::push_front<OptTypes0, std::string>::type OptTypes1;
typedef boost::mpl::push_front<OptTypes1, cv::Rect2i>::type OptTypes2;
typedef boost::mpl::push_front<OptTypes2, cv::Rect2f>::type OptTypes3;
typedef boost::mpl::push_front<OptTypes3, cv::Rect2d>::type OptTypes4;
typedef boost::mpl::push_front<OptTypes4, cv::Point2i>::type OptTypes5;
typedef boost::mpl::push_front<OptTypes5, cv::Point2f>::type OptTypes6;
typedef boost::mpl::push_front<OptTypes6, cv::Point2d>::type OptTypes7;
typedef boost::mpl::push_front<OptTypes7, cv::Point3i>::type OptTypes8;
typedef boost::mpl::push_front<OptTypes8, cv::Point3f>::type OptTypes9;
typedef boost::mpl::push_front<OptTypes9, cv::Point3d>::type OptTypes10;
typedef boost::mpl::push_front<OptTypes10, cv::Size2i>::type OptTypes11;
typedef boost::mpl::push_front<OptTypes11, cv::Size2l>::type OptTypes12;
typedef boost::mpl::push_front<OptTypes12, cv::Size2f>::type OptTypes13;
typedef boost::mpl::push_front<OptTypes13, cv::Size2d>::type OptTypes14;
typedef boost::mpl::push_front<OptTypes14, cv::Scalar>::type OptTypes15;
typedef boost::mpl::push_front<OptTypes15, cv::Scalar_<int>>::type OptTypes16;
typedef boost::mpl::push_front<OptTypes16, cv::Scalar_<long long>>::type OptTypes;
using AnyMap = std::map<std::string, boost::make_variant_over<OptTypes>::type>;

template<typename T>
void get_any_field(const AnyMap &fields, const std::string &n, T &v)
{
    auto it = fields.find(n);
    if (it != fields.cend())
    {
        auto p = boost::get<T>(&it->second);
        if (p)
        {
            v = *p;
        }
    }
}

#define SET_ANY_FIELD(fields, name) fields[BOOST_PP_STRINGIZE(name)] = name
#define GET_ANY_FIELD(fields, name) get_any_field(fields, BOOST_PP_STRINGIZE(name), name)

#endif
