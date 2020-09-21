#include "precomp.hpp"
#include "h5db_helper.h"
#include <array>
#include <boost/filesystem.hpp>

namespace cv {
namespace mvlab {

template <>
H5::PredType H5DBHelper::NativeType<bool>()
{
    return H5::PredType::NATIVE_HBOOL;
}

template <> 
H5::PredType H5DBHelper::NativeType<int>()
{ 
    return H5::PredType::NATIVE_INT;
}

template <> 
H5::PredType H5DBHelper::NativeType<long>()
{ 
    return H5::PredType::NATIVE_LONG;
}

template <>
H5::PredType H5DBHelper::NativeType<long long>()
{
    return H5::PredType::NATIVE_LLONG;
}

template <>
H5::PredType H5DBHelper::NativeType<float>()
{
    return H5::PredType::NATIVE_FLOAT;
}

template <>
H5::PredType H5DBHelper::NativeType<double>()
{
    return H5::PredType::NATIVE_DOUBLE;
}

template<>
cv::Scalar H5DBHelper::Load<cv::Scalar>(const H5::Group &g, const std::string &n, const cv::Scalar &d)
{
    if (g.nameExists(n))
    {
        H5::DataSet dataset = g.openDataSet(n);
        H5::DataType datatype = dataset.getDataType();

        hsize_t dimsm[1] = { 4 };
        H5::ArrayType arrType(H5::PredType::NATIVE_UCHAR, 1, dimsm);
        if (datatype.getClass() == arrType.getClass())
        {
            H5::DataSpace s = dataset.getSpace();
            if (H5S_SCALAR == s.getSimpleExtentType())
            {
                std::array<unsigned char, 4> val = { 0 };
                dataset.read(val.data(), arrType);
                return cv::Scalar(val[0], val[1], val[2], val[3]);
            }
        }
    }

    return d;
}

void H5DBHelper::Save(const H5::Group &g, const std::string &n, const cv::Scalar &c)
{
    if (g.nameExists(n))
    {
        H5Ldelete(g.getId(), n.data(), H5P_DEFAULT);
    }

    if (!g.nameExists(n))
    {
        hsize_t dimsm[1] = { 4 };
        H5::ArrayType arrType(H5::PredType::NATIVE_UCHAR, 1, dimsm);
        H5::DataSpace dataSpace(H5S_SCALAR);
        H5::DataSet dataSet = g.createDataSet(n, arrType, dataSpace);

        std::array<unsigned char, 4> d = { cv::saturate_cast<uchar>(c[0]), cv::saturate_cast<uchar>(c[1]), cv::saturate_cast<uchar>(c[2]), cv::saturate_cast<uchar>(c[3]) };
        dataSet.write(d.data(), arrType);
    }
}

void H5DBHelper::SetAttribute(const H5::H5Object &o, const std::string &n, const cv::Scalar &c)
{
    if (o.attrExists(n))
    {
        o.removeAttr(n);
    }

    if (!o.attrExists(n))
    {
        hsize_t dimsm[1] = { 4 };
        H5::ArrayType arrType(H5::PredType::NATIVE_UCHAR, 1, dimsm);
        H5::DataSpace attSpace(H5S_SCALAR);
        H5::Attribute att = o.createAttribute(n, arrType, attSpace);
        std::array<unsigned char, 4> d = { cv::saturate_cast<uchar>(c[0]), cv::saturate_cast<uchar>(c[1]), cv::saturate_cast<uchar>(c[2]), cv::saturate_cast<uchar>(c[3]) };
        att.write(arrType, d.data());
    }
}

void H5DBHelper::SetAttribute(const H5::H5Object &o, const std::string &n, const std::string &v)
{
    if (o.attrExists(n))
    {
        o.removeAttr(n);
    }

    if (!o.attrExists(n))
    {
        H5::StrType strType(H5::PredType::C_S1, H5T_VARIABLE);
        H5::DataSpace attSpace(H5S_SCALAR);
        H5::Attribute att = o.createAttribute(n, strType, attSpace);
        att.write(strType, v);
    }
}

template<>
std::string H5DBHelper::GetAttribute<std::string>(const H5::H5Object &o, const std::string &n)
{
    H5std_string v;
    if (o.attrExists(n))
    {
        H5::Attribute a = o.openAttribute(n);
        if (H5T_STRING == a.getTypeClass())
        {
            H5::DataSpace s = a.getSpace();
            if (H5S_SCALAR == s.getSimpleExtentType())
            {
                H5::StrType strType(H5::PredType::C_S1, H5T_VARIABLE);
                a.read(strType, v);
            }
        }
    }

    return v;
}

template<>
cv::Scalar H5DBHelper::GetAttribute<cv::Scalar>(const H5::H5Object &o, const std::string &n)
{
    std::array<unsigned char, 4> d = { 0 };
    if (o.attrExists(n))
    {
        H5::Attribute a = o.openAttribute(n);

        hsize_t dimsm[1] = { 4 };
        H5::ArrayType arrType(H5::PredType::NATIVE_UCHAR, 1, dimsm);
        if (arrType.getClass() == a.getTypeClass())
        {
            H5::DataSpace s = a.getSpace();
            if (H5S_SCALAR == s.getSimpleExtentType())
            {
                a.read(arrType, d.data());
            }
        }
    }

    return cv::Scalar(d[0], d[1], d[2], d[3]);
}

std::vector<cv::String> H5DBHelper::GetSpamProjects(const cv::String &dbPath)
{
    boost::filesystem::path p(dbPath);
    boost::system::error_code ec;
    std::vector<cv::String> projs;
    if (boost::filesystem::exists(p, ec) && boost::filesystem::is_regular_file(p, ec))
    {
        try 
        {
            H5::Exception::dontPrint();
            H5std_string ansiDBPath = dbPath;
            H5::H5File f(ansiDBPath, H5F_ACC_RDONLY);
            H5::Group g = f.openGroup("/");

            auto numObjs = g.getNumObjs();
            for (hsize_t i=0; i<numObjs; ++i)
            {
                H5std_string tName;
                auto oType = g.getObjTypeByIdx(i, tName);
                if (H5G_GROUP == oType)
                {
                    H5std_string gName = g.getObjnameByIdx(i);
                    H5::Group gProj = g.openGroup(gName);
                    projs.push_back(gName);
                }
            }
        }
        catch (const H5::Exception &e)
        {
            return projs;
        }
    }

    return projs;
}

}
}