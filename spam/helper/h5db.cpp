#include "h5db.h"
#include "commondef.h"
#include <array>
#include <wx/log.h>
#include <boost/filesystem.hpp>

template <>
H5::PredType H5DB::NativeType<bool>()
{
    return H5::PredType::NATIVE_HBOOL;
}

template <> 
H5::PredType H5DB::NativeType<int>()
{ 
    return H5::PredType::NATIVE_INT;
}

template <> 
H5::PredType H5DB::NativeType<long>()
{ 
    return H5::PredType::NATIVE_LONG;
}

template <>
H5::PredType H5DB::NativeType<float>()
{
    return H5::PredType::NATIVE_FLOAT;
}

template <>
H5::PredType H5DB::NativeType<double>()
{
    return H5::PredType::NATIVE_DOUBLE;
}

template<>
wxColour H5DB::Load<wxColour>(const H5::Group &g, const std::string &n)
{
    std::array<unsigned char, 4> d = { 0 };
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
                dataset.read(d.data(), arrType);
            }
        }
    }

    return wxColour(d[0], d[1], d[2], d[3]);
}

void H5DB::Save(const H5::Group &g, const std::string &n, const wxColour &c)
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

        std::array<unsigned char, 4> d = { c.Red(), c.Green(), c.Blue(), c.Alpha() };
        dataSet.write(d.data(), arrType);
    }
}

void H5DB::Save(const H5::Group &g, const std::string &n, const std::vector<std::array<double, 2>> &points)
{
    if (g.nameExists(n))
    {
        H5Ldelete(g.getId(), n.data(), H5P_DEFAULT);
    }

    if (!g.nameExists(n))
    {
        hsize_t dimsm[1] = { 2 };
        H5::ArrayType arrType(H5::PredType::NATIVE_DOUBLE, 1, dimsm);

        int rank = 1;
        const hsize_t dims[1] = { points.size() };
        H5::DataSpace dataSpace(rank, dims);
        H5::DataSet dataSet = g.createDataSet(n, arrType, dataSpace);

        dataSet.write(&points.front()[0], arrType);
    }
}

void H5DB::Save(const H5::Group &g, const std::string &n, const std::array<double, 6> &transformMat)
{
    if (g.nameExists(n))
    {
        H5Ldelete(g.getId(), n.data(), H5P_DEFAULT);
    }

    if (!g.nameExists(n))
    {
        hsize_t dimsm[1] = { 6 };
        H5::DataSpace dataSpace(1, dimsm);
        H5::DataSet dataSet = g.createDataSet(n, H5::PredType::NATIVE_DOUBLE, dataSpace);

        dataSet.write(transformMat.data(), H5::PredType::NATIVE_DOUBLE);
    }
}

bool H5DB::Load(const H5::Group &g, const std::string &n, std::vector<std::array<double, 2>> &points)
{
    points.clear();
    if (g.nameExists(n))
    {
        H5::DataSet dataset = g.openDataSet(n);
        H5::DataType datatype = dataset.getDataType();

        hsize_t dimsm[1] = { 2 };
        H5::ArrayType arrType(H5::PredType::NATIVE_DOUBLE, 1, dimsm);
        if (datatype.getClass() == arrType.getClass())
        {
            H5::DataSpace s = dataset.getSpace();
            if (H5S_SIMPLE == s.getSimpleExtentType() &&
                1 == s.getSimpleExtentNdims())
            {
                hsize_t dims[1] = { 0 };
                s.getSimpleExtentDims(dims);
                H5::ArrayType aType = dataset.getArrayType();
                if (1 == aType.getArrayNDims() && dims[0]>0)
                {
                    dimsm[0] = 0;
                    aType.getArrayDims(dimsm);
                    if (2 == dimsm[0])
                    {
                        points.resize(dims[0]);
                        dataset.read(points.data()->data(), aType);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool H5DB::Load(const H5::Group &g, const std::string &n, std::array<double, 6> &transformMat)
{
    if (g.nameExists(n))
    {
        H5::DataSet dataset = g.openDataSet(n);
        H5::DataType datatype = dataset.getDataType();

        hsize_t dims[1] = { 6 };
        const H5::PredType &elemType = H5::PredType::NATIVE_DOUBLE;
        if (datatype.getClass() == elemType.getClass())
        {
            H5::DataSpace s = dataset.getSpace();
            if (H5S_SIMPLE == s.getSimpleExtentType() &&
                1 == s.getSimpleExtentNdims())
            {
                hsize_t dims[1] = { 0 };
                s.getSimpleExtentDims(dims);
                if (6 == dims[0])
                {
                    dataset.read(transformMat.data(), elemType);
                    return true;
                }
            }
        }
    }

    return false;
}

void H5DB::SetAttribute(const H5::H5Object &o, const std::string &n, const wxColour &c)
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

        std::array<unsigned char, 4> d = { c.Red(), c.Green(), c.Blue(), c.Alpha() };
        att.write(arrType, d.data());
    }
}

void H5DB::SetAttribute(const H5::H5Object &o, const std::string &n, const std::string &v)
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
std::string H5DB::GetAttribute<std::string>(const H5::H5Object &o, const std::string &n)
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
wxColour H5DB::GetAttribute<wxColour>(const H5::H5Object &o, const std::string &n)
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

    return wxColour(d[0], d[1], d[2], d[3]);
}

std::vector<wxString> H5DB::GetSpamProjects(const wxString &dbPath)
{
    boost::filesystem::path p(dbPath.ToStdWstring());
    boost::system::error_code ec;
    std::vector<wxString> projs;
    if (boost::filesystem::exists(p, ec) && boost::filesystem::is_regular_file(p, ec))
    {
        try 
        {
            H5::Exception::dontPrint();
            H5std_string ansiDBPath = dbPath.ToStdString();
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

                    const auto &v = GetAttribute<std::string>(gProj, CommonDef::GetSpamDBNodeTypeAttrName());
                    if (v== CommonDef::GetProjNodeTypeName())
                    {
                        wxString wProjName = wxString::FromUTF8(gName);
                        projs.push_back(wProjName);
                    }
                }
            }
        }
        catch (const H5::Exception &e)
        {
            wxLogError(e.getCDetailMsg());
            return projs;
        }
    }

    return projs;
}