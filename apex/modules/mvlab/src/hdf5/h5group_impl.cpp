#include "precomp.hpp"
#include "h5db_helper.h"
#include "h5group_impl.hpp"
#include <region/region_impl.hpp>
#include <region/region_array_impl.hpp>
#include <matching/pixel_template_impl.hpp>
#include <matching/contour_template_impl.hpp>
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

bool H5GroupImpl::Valid() const
{
    return H5::IdComponent::isValid(group_.getId());
}

cv::String H5GroupImpl::GetErrorStatus() const
{
    return err_msg_;
}

cv::Ptr<H5Group> H5GroupImpl::GetGroup(const cv::String &name) const
{
    try
    {
        H5::Exception::dontPrint();
        if (group_.nameExists(name))
        {
            return makePtr<H5GroupImpl>(group_.openGroup(name));
        }
        else
        {
            H5::LinkCreatPropList lcpl;
            lcpl.setCharEncoding(H5T_CSET_UTF8);
            return makePtr<H5GroupImpl>(group_.createGroup(name, lcpl));
        }
    }
    catch (const H5::Exception &e)
    {
        return makePtr<H5GroupImpl>(e.getDetailMsg());
    }
}

int H5GroupImpl::SetInt(const cv::String &name, const int val)
{
    try
    {
        H5::Exception::dontPrint();
        if (H5GroupImpl::Valid())
        {
            H5DBHelper::Save(group_, name, val);
            return MLR_SUCCESS;
        }
        else
        {
            return MLR_H5DB_INVALID;
        }
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }
}

int H5GroupImpl::SetMat(const cv::String &name, const cv::Mat &val)
{
    H5::DataSet dataSet;
    return SetDataSet(name, val, dataSet);
}

int H5GroupImpl::SetRegion(const cv::String &name, const cv::Ptr<Region> &val)
{
    if (!val || val->Empty())
    {
        return MLR_REGION_EMPTY;
    }

    if (H5GroupImpl::Valid())
    {
        const cv::Ptr<RegionImpl> rgn = val.dynamicCast<RegionImpl>();
        if (rgn)
        {
            return rgn->Serialize(name, this);
        }
        else
        {
            const cv::Ptr<RegionArrayImpl> rgns = val.dynamicCast<RegionArrayImpl>();
            if (rgns)
            {
                return rgns->Serialize(name, this);
            }

            return MLR_MEMORY_ERROR;
        }
    }
    else
    {
        return MLR_H5DB_INVALID;
    }
}

int H5GroupImpl::SetContour(const cv::String &name, const cv::Ptr<Contour> &val)
{
    if (!val || val->Empty())
    {
        return MLR_REGION_EMPTY;
    }

    if (H5GroupImpl::Valid())
    {
        const cv::Ptr<ContourImpl> contour = val.dynamicCast<ContourImpl>();
        if (contour)
        {
            return contour->Serialize(name, this);
        }
        else
        {
            const cv::Ptr<ContourArrayImpl> contours = val.dynamicCast<ContourArrayImpl>();
            if (contours)
            {
                return contours->Serialize(name, this);
            }

            return MLR_MEMORY_ERROR;
        }
    }
    else
    {
        return MLR_H5DB_INVALID;
    }
}

int H5GroupImpl::SetContourTemplate(const cv::String &name, const cv::Ptr<ContourTemplate> &val)
{
    err_msg_.resize(0);
    if (!H5GroupImpl::Valid())
    {
        err_msg_ = "invalid database";
        return MLR_H5DB_INVALID;
    }

    cv::Ptr<ContourTemplateImpl> tmpl = val.dynamicCast<ContourTemplateImpl>();
    if (!tmpl || tmpl->Empty())
    {
        err_msg_ = "empty contour template";
        return MLR_TEMPLATE_EMPTY;
    }

    try
    {
        int r = tmpl->Serialize(name, this);
        err_msg_ = tmpl->GetErrorStatus();
        return r;
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }

    return MLR_SUCCESS;
}

int H5GroupImpl::SetPixelTemplate(const cv::String &name, const cv::Ptr<PixelTemplate> &val)
{
    err_msg_.resize(0);
    if (!H5GroupImpl::Valid())
    {
        err_msg_ = "invalid database";
        return MLR_H5DB_INVALID;
    }

    cv::Ptr<PixelTemplateImpl> tmpl = val.dynamicCast<PixelTemplateImpl>();
    if (!tmpl || tmpl->Empty())
    {
        err_msg_ = "empty pixel template";
        return MLR_TEMPLATE_EMPTY;
    }

    try
    {
        int r = tmpl->Serialize(name, this);
        err_msg_ = tmpl->GetErrorStatus();
        return r;
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }

    return MLR_SUCCESS;
}

int H5GroupImpl::GetInt(const cv::String &name, const int defaultVal) const
{
    try
    {
        H5::Exception::dontPrint();
        if (H5GroupImpl::Valid())
        {
            return H5DBHelper::Load<int>(group_, name, defaultVal);
        }
        else
        {
            return defaultVal;
        }
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return defaultVal;
    }
}

cv::Mat H5GroupImpl::GetMat(const cv::String &name) const
{
    cv::Mat mat;
    H5::DataSet dataSet;
    GetDataSet(name, mat, dataSet);

    return mat;
}

cv::Ptr<Region> H5GroupImpl::GetRegion(const cv::String &name) const
{
    std::string bytes;
    H5::DataSet dataSet;
    if (MLR_SUCCESS == GetDataSet(name, RegionImpl::TypeGUID(), bytes, dataSet))
    {
        return cv::makePtr<RegionImpl>(bytes);
    }
    else if (MLR_SUCCESS == GetDataSet(name, RegionArrayImpl::TypeGUID(), bytes, dataSet))
    {
        return cv::makePtr<RegionArrayImpl>(bytes);
    }
    else
    {
        return cv::makePtr<RegionImpl>();
    }
}

cv::Ptr<Contour> H5GroupImpl::GetContour(const cv::String &name) const
{
    std::string bytes;
    H5::DataSet dataSet;
    if (MLR_SUCCESS == GetDataSet(name, ContourImpl::TypeGUID(), bytes, dataSet))
    {
        return cv::makePtr<ContourImpl>(bytes);
    }
    else if (MLR_SUCCESS == GetDataSet(name, ContourArrayImpl::TypeGUID(), bytes, dataSet))
    {
        return cv::makePtr<ContourArrayImpl>(bytes);
    }
    else
    {
        return cv::makePtr<ContourImpl>();
    }
}

cv::Ptr<ContourTemplate> H5GroupImpl::GetContourTemplate(const cv::String &name)
{
    std::string bytes;
    H5::DataSet dataSet;
    err_msg_.resize(0);
    if (MLR_SUCCESS == GetDataSet(name, ContourTemplateImpl::TypeGUID(), bytes, dataSet))
    {
        return cv::makePtr<ContourTemplateImpl>(bytes);
    }
    else
    {
        return cv::makePtr<ContourTemplateImpl>();
    }
}

cv::Ptr<PixelTemplate> H5GroupImpl::GetPixelTemplate(const cv::String &name)
{
    std::string bytes;
    H5::DataSet dataSet;
    err_msg_.resize(0);
    if (MLR_SUCCESS == GetDataSet(name, PixelTemplateImpl::TypeGUID(), bytes, dataSet))
    {
        return cv::makePtr<PixelTemplateImpl>(bytes);
    }
    else
    {
        return cv::makePtr<PixelTemplateImpl>();
    }
}

H5::PredType H5GroupImpl::GetH5type(const int cvType) const
{
    switch (CV_MAT_DEPTH(cvType))
    {
    case CV_64F:
        return H5::PredType::NATIVE_DOUBLE;
    case CV_32F:
        return H5::PredType::NATIVE_FLOAT;
    case CV_8U:
        return H5::PredType::NATIVE_UINT8;
    case CV_8S:
        return H5::PredType::NATIVE_INT8;
    case CV_16U:
        return H5::PredType::NATIVE_UINT16;
    case CV_16S:
        return H5::PredType::NATIVE_INT16;
    case CV_32S:
        return H5::PredType::NATIVE_INT32;
    default:
        break;
    }

    return H5::PredType::NATIVE_OPAQUE;
}

int H5GroupImpl::GetCVtype(const H5::DataType &h5Type) const
{
    if (h5Type.operator==(H5::PredType::NATIVE_UINT8))
    {
        return CV_8U;
    }
    else if (h5Type.operator==(H5::PredType::NATIVE_INT8))
    {
        return CV_8S;
    }
    else if (h5Type.operator==(H5::PredType::NATIVE_UINT16))
    {
        return CV_16U;
    }
    else if (h5Type.operator==(H5::PredType::NATIVE_INT16))
    {
        return CV_16S;
    }
    else if (h5Type.operator==(H5::PredType::NATIVE_INT32))
    {
        return CV_32S;
    }
    else if (h5Type.operator==(H5::PredType::NATIVE_FLOAT))
    {
        return CV_32F;
    }
    else if (h5Type.operator==(H5::PredType::NATIVE_DOUBLE))
    {
        return CV_64F;
    }
    else
    {
        return -1;
    }
}

int H5GroupImpl::SetDataSet(const cv::String &name, const cv::Mat &val, H5::DataSet &dataSet)
{
    if (val.empty())
    {
        return MLR_IMAGE_EMPTY;
    }

    if (!H5GroupImpl::Valid())
    {
        return MLR_H5DB_INVALID;
    }

    cv::Mat mat = val;
    if (!mat.isContinuous())
    {
        mat = mat.clone();
    }

    if (!mat.isContinuous())
    {
        return MLR_IMAGE_STORAGE_NOT_CONTINUOUS;
    }

    const int ndims = mat.dims;
    std::vector<hsize_t> dim_vec(ndims);
    for (int i = 0; i < ndims; i++)
    {
        dim_vec[i] = mat.size[i];
    }

    H5::PredType dtype = GetH5type(mat.type());
    if (H5::PredType::NATIVE_OPAQUE == dtype)
    {
        return MLR_IMAGE_FORMAT_ERROR;
    }

    try
    {
        H5::Exception::dontPrint();
        if (group_.nameExists(name))
        {
            H5Ldelete(group_.getId(), name.data(), H5P_DEFAULT);
        }

        H5::DataSpace dataSpace(ndims, dim_vec.data());
        if (mat.channels() > 1)
        {
            hsize_t dimsm[1] = { static_cast<hsize_t>(mat.channels()) };
            H5::ArrayType arrType(dtype, 1, dimsm);
            dataSet = group_.createDataSet(name, arrType, dataSpace);
            dataSet.write(mat.data, arrType);
        }
        else
        {
            dataSet = group_.createDataSet(name, dtype, dataSpace);
            dataSet.write(mat.data, dtype);
        }

        return MLR_SUCCESS;
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }
}

int H5GroupImpl::SetDataSet(const cv::String &name, const std::string &bytes, H5::DataSet &dataSet)
{
    constexpr long long cols = 32;
    const long long datasize = static_cast<long long>(bytes.size());
    const long long arraysize = (datasize + cols - 1) & (-cols);
    const long long rows = arraysize / cols;
    if (!rows)
    {
        return MLR_DATA_EMPTY;
    }

    cv::Mat mat = cv::Mat::zeros(rows, cols, CV_8UC1);
    if (mat.empty())
    {
        return MLR_MEMORY_ERROR;
    }

    std::memcpy(mat.data, bytes.data(), datasize);

    int r = SetDataSet(name, mat, dataSet);
    if (MLR_SUCCESS == r && H5::IdComponent::isValid(dataSet.getId()))
    {
        return SetAttribute(dataSet, cv::String("FlatSize"), datasize);
    }

    return r;
}

int H5GroupImpl::GetDataSet(const cv::String &name, const cv::String &typeGUID, std::string &bytes, H5::DataSet &dataSet) const
{
    cv::Mat mat;
    bytes.resize(0);
    err_msg_.resize(0);
    int r = GetDataSet(name, typeGUID, mat, dataSet);
    if (MLR_SUCCESS == r &&
        H5::IdComponent::isValid(dataSet.getId()) &&
        !mat.empty() &&
        CV_8U == mat.depth() &&
        1 == mat.channels() &&
        2 == mat.dims &&
        mat.isContinuous())
    {
        long long datasize = 0;
        r = GetAttribute(dataSet, cv::String("FlatSize"), datasize);
        if (MLR_SUCCESS == r && datasize && datasize <= mat.rows * mat.cols)
        {
            bytes.resize(static_cast<std::string::size_type>(datasize));
            std::memcpy(bytes.data(), mat.data, datasize);
            return MLR_SUCCESS;
        }
    }

    err_msg_ = "open dataset error";
    return MLR_H5DB_INVALID;
}

int H5GroupImpl::GetDataSet(const cv::String &name, cv::Mat &val, H5::DataSet &dataSet) const
{
    return GetDataSet(name, cv::String(), val, dataSet);
}

int H5GroupImpl::GetDataSet(const cv::String &name, const cv::String &typeGUID, cv::Mat &val, H5::DataSet &dataSet) const
{
    try
    {
        if (!H5GroupImpl::Valid())
        {
            return MLR_H5DB_INVALID;
        }

        if (!group_.nameExists(name))
        {
            return MLR_H5DB_NAME_NOT_EXIST;
        }

        H5::Exception::dontPrint();
        dataSet = group_.openDataSet(name);
        if (!typeGUID.empty())
        {
            cv::String strType;
            int r = GetAttribute(dataSet, cv::String("TypeGUID"), strType);
            if (MLR_SUCCESS != r)
            {
                return r;
            }

            if (strType != typeGUID)
            {
                return MLR_H5DB_TYPE_NOT_MATCH;
            }
        }

        H5::DataSpace s = dataSet.getSpace();
        if (H5S_SIMPLE != s.getSimpleExtentType())
        {
            return MLR_H5DB_TYPE_NOT_MATCH;
        }

        const int numDims = s.getSimpleExtentNdims();
        std::vector<hsize_t> h5DimSize(numDims);
        if (numDims != s.getSimpleExtentDims(h5DimSize.data(), nullptr))
        {
            return MLR_H5DB_INVALID;
        }
        std::vector<int> dimSizes(h5DimSize.size());
        std::transform(h5DimSize.cbegin(), h5DimSize.cend(), dimSizes.begin(), [](const hsize_t v) { return static_cast<int>(v); });

        if (H5T_ARRAY == dataSet.getTypeClass())
        {
            H5::ArrayType arrType = dataSet.getArrayType();
            H5::DataType dataType = arrType.getSuper();
            if (1 != arrType.getArrayNDims()) { return MLR_H5DB_TYPE_NOT_MATCH; }
            const int cvType = GetCVtype(dataType);
            if (-1 == cvType) { return MLR_H5DB_TYPE_NOT_MATCH; }

            hsize_t dimsm[1]{ 0 };
            arrType.getArrayDims(dimsm);
            const int numChannels = dimsm[0];

            val.create(numDims, dimSizes.data(), CV_MAKETYPE(cvType, numChannels));
            dataSet.read(val.data, arrType);
        }
        else
        {
            H5::DataType dataType = dataSet.getDataType();
            const int cvType = GetCVtype(dataType);
            if (-1 == cvType) { return MLR_H5DB_TYPE_NOT_MATCH; }

            val.create(numDims, dimSizes.data(), cvType);
            dataSet.read(val.data, dataType);
        }

        return MLR_SUCCESS;
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }
}

int H5GroupImpl::SetAttribute(const H5::H5Object &h5o, const cv::String &name, const int val)
{
    try
    {
        H5::Exception::dontPrint();
        if (H5::H5File::isValid(h5o.getId()))
        {
            H5DBHelper::SetAttribute(h5o, name, val);
            return MLR_SUCCESS;
        }
        else
        {
            return MLR_H5DB_INVALID;
        }
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }
}

int H5GroupImpl::GetAttribute(const H5::H5Object &h5o, const cv::String &name, int &val) const
{
    try
    {
        H5::Exception::dontPrint();
        if (H5::H5File::isValid(h5o.getId()))
        {
            val = H5DBHelper::GetAttribute<int>(h5o, name);
            return MLR_SUCCESS;
        }
        else
        {
            return MLR_H5DB_INVALID;
        }
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }
}

int H5GroupImpl::SetAttribute(const H5::H5Object &h5o, const cv::String &name, const long long val)
{
    try
    {
        H5::Exception::dontPrint();
        if (H5::H5File::isValid(h5o.getId()))
        {
            H5DBHelper::SetAttribute(h5o, name, val);
            return MLR_SUCCESS;
        }
        else
        {
            return MLR_H5DB_INVALID;
        }
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }
}

int H5GroupImpl::GetAttribute(const H5::H5Object &h5o, const cv::String &name, long long &val) const
{
    try
    {
        H5::Exception::dontPrint();
        if (H5::H5File::isValid(h5o.getId()))
        {
            val = H5DBHelper::GetAttribute<long long>(h5o, name);
            return MLR_SUCCESS;
        }
        else
        {
            return MLR_H5DB_INVALID;
        }
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }
}

int H5GroupImpl::SetAttribute(const H5::H5Object &h5o, const cv::String &name, const cv::String &val)
{
    try
    {
        H5::Exception::dontPrint();
        if (H5::H5File::isValid(h5o.getId()))
        {
            H5DBHelper::SetAttribute(h5o, name, val);
            return MLR_SUCCESS;
        }
        else
        {
            return MLR_H5DB_INVALID;
        }
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }
}

int H5GroupImpl::GetAttribute(const H5::H5Object &h5o, const cv::String &name, cv::String &val) const
{
    try
    {
        H5::Exception::dontPrint();
        if (H5::H5File::isValid(h5o.getId()))
        {
            val = H5DBHelper::GetAttribute<cv::String>(h5o, name);
            return MLR_SUCCESS;
        }
        else
        {
            return MLR_H5DB_INVALID;
        }
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }
}

int H5GroupImpl::SetAttribute(const H5::H5Object &h5o, const cv::String &name, const cv::Mat &val)
{
    if (val.empty())
    {
        return MLR_IMAGE_EMPTY;
    }

    if (!H5::H5File::isValid(h5o.getId()))
    {
        return MLR_H5DB_INVALID;
    }

    cv::Mat mat = val;
    if (!mat.isContinuous())
    {
        mat = mat.clone();
    }

    if (!mat.isContinuous())
    {
        return MLR_IMAGE_STORAGE_NOT_CONTINUOUS;
    }

    const int ndims = mat.dims;
    std::vector<hsize_t> dim_vec(ndims);
    for (int i = 0; i < ndims; i++)
    {
        dim_vec[i] = mat.size[i];
    }

    H5::PredType dtype = GetH5type(mat.type());
    if (H5::PredType::NATIVE_OPAQUE == dtype)
    {
        return MLR_IMAGE_FORMAT_ERROR;
    }

    try
    {
        H5::Exception::dontPrint();
        if (h5o.attrExists(name))
        {
            h5o.removeAttr(name);
        }

        H5::DataSpace dataSpace(ndims, dim_vec.data());
        if (mat.channels() > 1)
        {
            hsize_t dimsm[1] = { static_cast<hsize_t>(mat.channels()) };
            H5::ArrayType arrType(dtype, 1, dimsm);
            H5::Attribute att = h5o.createAttribute(name, arrType, dataSpace);
            att.write(arrType, mat.data);
        }
        else
        {
            H5::Attribute att = h5o.createAttribute(name, dtype, dataSpace);
            att.write(dtype, mat.data);
        }

        return MLR_SUCCESS;
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }
}

int H5GroupImpl::GetAttribute(const H5::H5Object &h5o, const cv::String &name, cv::Mat &val) const
{
    try
    {
        if (!H5::H5File::isValid(h5o.getId()))
        {
            return MLR_H5DB_INVALID;
        }

        if (!h5o.attrExists(name))
        {
            return MLR_H5DB_NAME_NOT_EXIST;
        }

        H5::Attribute attri = h5o.openAttribute(name);
        H5::DataSpace s = attri.getSpace();
        if (H5S_SIMPLE != s.getSimpleExtentType())
        {
            return MLR_H5DB_TYPE_NOT_MATCH;
        }

        const int numDims = s.getSimpleExtentNdims();
        std::vector<hsize_t> h5DimSize(numDims);
        if (numDims != s.getSimpleExtentDims(h5DimSize.data(), nullptr))
        {
            return MLR_H5DB_INVALID;
        }
        std::vector<int> dimSizes(h5DimSize.size());
        std::transform(h5DimSize.cbegin(), h5DimSize.cend(), dimSizes.begin(), [](const hsize_t v) { return static_cast<int>(v); });

        if (H5T_ARRAY == attri.getTypeClass())
        {
            H5::ArrayType arrType = attri.getArrayType();
            H5::DataType dataType = arrType.getSuper();
            if (1 != arrType.getArrayNDims()) { return MLR_H5DB_TYPE_NOT_MATCH; }
            const int cvType = GetCVtype(dataType);
            if (-1 == cvType) { return MLR_H5DB_TYPE_NOT_MATCH; }

            hsize_t dimsm[1]{ 0 };
            arrType.getArrayDims(dimsm);
            const int numChannels = dimsm[0];

            val.create(numDims, dimSizes.data(), CV_MAKETYPE(cvType, numChannels));
            attri.read(arrType, val.data);
        }
        else
        {
            H5::DataType dataType = attri.getDataType();
            const int cvType = GetCVtype(dataType);
            if (-1 == cvType) { return MLR_H5DB_TYPE_NOT_MATCH; }

            val.create(numDims, dimSizes.data(), cvType);
            attri.read(dataType, val.data);
        }

        return MLR_SUCCESS;
    }
    catch (const H5::Exception &e)
    {
        err_msg_ = e.getDetailMsg();
        return MLR_H5DB_EXCEPTION;
    }
}

}
}
