#ifndef __OPENCV_MVLAB_COMMON_DEFINE_HPP__
#define __OPENCV_MVLAB_COMMON_DEFINE_HPP__

#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

constexpr int MLR_OK = 0;
constexpr int MLR_SUCCESS = MLR_OK;
constexpr int MLR_CPU_UNSUPPORTED = 1;
constexpr int MLR_MEMORY_ERROR = 2;
constexpr int MLR_SMART_PTR_EMPTY = 3;
constexpr int MLR_FEATURE_UNSUPPORTED = 4;
constexpr int MLR_FILESYSTEM_EXCEPTION = 5;
constexpr int MLR_ARCHIVE_EXCEPTION = 6;
constexpr int MLR_IO_STREAM_EXCEPTION = 7;
constexpr int MLR_DATA_EMPTY = 8;
constexpr int MLR_PARAMETER_ERROR = 5000;
constexpr int MLR_PARAMETER_ERROR_FILE_PATH = 5001;
constexpr int MLR_PARAMETER_ERROR_EXISTING_FILE = 5002;
constexpr int MLR_PARAMETER_ERROR_EXISTING_DIRECTORY = 5003;
constexpr int MLR_PARAMETER_ERROR_FILE_NOT_EXISTS = 5004;
constexpr int MLR_PARAMETER_ERROR_GUARD = 5100;
constexpr int MLR_IMAGE_EMPTY = 10000;
constexpr int MLR_IMAGE_FORMAT_ERROR = 10001;
constexpr int MLR_IMAGE_STORAGE_NOT_CONTINUOUS = 10002;
constexpr int MLR_REGION_EMPTY = 11000;
constexpr int MLR_CONTOUR_EMPTY = 12000;
constexpr int MLR_TEMPLATE_EMPTY = 13000;
constexpr int MLR_TEMPLATE_EMPTY_TEMPL_REGION = 13001;
constexpr int MLR_TEMPLATE_REGION_TOO_SMALL = 13002;
constexpr int MLR_TEMPLATE_REGION_TOO_LARGE = 13003;
constexpr int MLR_TEMPLATE_REGION_OUT_OF_RANGE = 13004;
constexpr int MLR_TEMPLATE_PYRAMID_LEVEL_INVALID = 13005;
constexpr int MLR_TEMPLATE_PYRAMID_LEVEL_TOO_LARGE = 13006;
constexpr int MLR_TEMPLATE_ANGLE_RANGE_INVALID = 13007;
constexpr int MLR_TEMPLATE_INSIGNIFICANT = 13008;
constexpr int MLR_TEMPLATE_CORRUPTED_DATA = 13009;
constexpr int MLR_TEMPLATE_INSTANCE_NOT_FOUND = 13010;
constexpr int MLR_H5DB_INVALID = 14000;
constexpr int MLR_H5DB_EXCEPTION = 14001;
constexpr int MLR_H5DB_NAME_NOT_EXIST = 14002;
constexpr int MLR_H5DB_TYPE_NOT_MATCH = 14003;
constexpr int MLR_ERROR = -1;
constexpr int MLR_FAILURE = MLR_ERROR;

enum BoundaryLineStyle 
{
    BOUNDARY_LINE_SOLID = 0,
    BOUNDARY_LINE_DASH = 1,
    BOUNDARY_LINE_DOT = 2,
    BOUNDARY_LINE_DASHDOT = 3,
    BOUNDARY_LINE_DASHDOTDOT = 4,
    BOUNDARY_LINE_CUSTOM = 5
};

struct CV_EXPORTS_W_SIMPLE FitLineParameters
{
    CV_WRAP FitLineParameters();
    CV_PROP_RW cv::String algorithm;
    CV_PROP_RW int        maxNumPoints;
    CV_PROP_RW int        clippingEndPoints;
    CV_PROP_RW int        iterations;
    CV_PROP_RW float      clippingFactor;
};

struct CV_EXPORTS_W_SIMPLE FitLineResults
{
    CV_WRAP FitLineResults();
    CV_PROP_RW double xBegin;
    CV_PROP_RW double yBegin;
    CV_PROP_RW double xEnd;
    CV_PROP_RW double yEnd;
    CV_PROP_RW double nx;
    CV_PROP_RW double ny;
    CV_PROP_RW double dist;
};

}
}

#endif //__OPENCV_MVLAB_COMMON_DEFINE_HPP__
