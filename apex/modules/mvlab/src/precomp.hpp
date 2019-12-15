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
#pragma warning( push )
#pragma warning( disable : 4389 4310 )
#include <vectorclass.h>
#pragma warning( pop )
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>
#include <cairomm/cairomm.h>
#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
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
constexpr float D_XY_ONE = 1 << XY_SHIFT;
constexpr int K_NO = 0;
constexpr int K_YES = 1;
constexpr int K_UNKNOWN = 2;

template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Point> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Point2f> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Point2d> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Rect> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Rect2f> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Rect2d> : boost::true_type {};
template <> struct boost::optional_config::optional_uses_direct_storage_for<cv::Scalar> : boost::true_type {};

using Point2fSequence                   = std::vector<cv::Point2f>;
using ScalableIntSequence               = std::vector<int, MyAlloc<int>>;
using ScalableIntSequenceSequence       = std::vector<ScalableIntSequence, MyAlloc<ScalableIntSequence>>;
using ScalableDoubleSequence            = std::vector<double, MyAlloc<double>>;
using ScalablePoint2fSequence           = std::vector<cv::Point2f, MyAlloc<cv::Point2f>>;
using ScalablePoint2dSequence           = std::vector<cv::Point2d, MyAlloc<cv::Point2d>>;
using ScalableRectSequence              = std::vector<cv::Rect, MyAlloc<cv::Rect>>;
using ScalablePoint2fSequenceSequence   = std::vector<ScalablePoint2fSequence, MyAlloc<ScalablePoint2fSequence>>;
using UScalableBoolSequence             = ao::uvector<bool, MyAlloc<bool>>;
using UScalableIntSequence              = ao::uvector<int, MyAlloc<int>>;
using UScalableFloatSequence            = ao::uvector<float, MyAlloc<float>>;
using UScalableDoubleSequence           = ao::uvector<double, MyAlloc<double>>;
using UScalableUCharSequence            = ao::uvector<uint8_t, MyAlloc<uint8_t>>;
using UScalablePointSequence            = ao::uvector<cv::Point, MyAlloc<cv::Point>>;
using UScalablePoint2fSequence          = ao::uvector<cv::Point2f, MyAlloc<cv::Point2f>>;
using UScalablePoint2dSequence          = ao::uvector<cv::Point2d, MyAlloc<cv::Point2d>>;
using UScalableRectSequence             = ao::uvector<cv::Rect, MyAlloc<cv::Rect>>;

#endif
