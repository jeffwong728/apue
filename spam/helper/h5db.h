#ifndef SPAM_HELPER_H5DB_H
#define SPAM_HELPER_H5DB_H
#include <boost/core/noncopyable.hpp>
#include <array>
#include <vector>
#include <type_traits>
#include <wx/colour.h>
#include <H5Cpp.h>
#include <hdf5_hl.h>

class H5DB : private boost::noncopyable
{
public:
    H5DB() = delete;

private:
    template<typename T> static void Save_impl(const H5::Group &g, const std::string &n, const T &v, std::true_type);
    template<typename T> static T Load_impl(const H5::Group &g, const std::string &n, std::true_type);
    template<typename T> static void SetAttribute_impl(const H5::H5Object &o, const std::string &n, const T &v, std::true_type);
    template<typename T> static T GetAttribute_impl(const H5::H5Object &g, const std::string &n, std::true_type);

public:
    static void Save(const H5::Group &g, const std::string &n, const wxColour &c);
    static void SetAttribute(const H5::H5Object &o, const std::string &n, const wxColour &c);
    static void SetAttribute(const H5::H5Object &o, const std::string &n, const std::string &v);
    template<typename T> static H5::PredType NativeType();  
    template<typename T> static void Save(const H5::Group &g, const std::string &n, const T &v);
    template<typename T> static void Save(const H5::Group &g, const std::string &n, const std::vector<T> &seq);
    template<typename T, std::size_t Dim> static void Save(const H5::Group &g, const std::string &n, const std::vector<std::array<T, Dim>> &points);
    template<typename T, std::size_t Dim> static void Save(const H5::Group &g, const std::string &n, const std::array<T, Dim> &arr);
    template<typename T, std::size_t Dim> static bool Load(const H5::Group &g, const std::string &n, std::array<T, Dim> &arr);
    template<typename T> static T Load(const H5::Group &g, const std::string &n);
    template<typename T> static bool Load(const H5::Group &g, const std::string &n, std::vector<T> &seq);
    template<typename T, std::size_t Dim> static bool Load(const H5::Group &g, const std::string &n, std::vector<std::array<T, Dim>> &points);
    template<typename T> static void SetAttribute(const H5::H5Object &o, const std::string &n, const T &v);
    template<typename T> static T GetAttribute(const H5::H5Object &o, const std::string &n);
    static std::vector<wxString> GetSpamProjects(const wxString &dbPath);
};

template<typename T>
void H5DB::Save_impl(const H5::Group &g, const std::string &n, const T &v, std::true_type)
{
    if (g.nameExists(n))
    {
        H5Ldelete(g.getId(), n.data(), H5P_DEFAULT);
    }
    if (!g.nameExists(n))
    {
        H5::DataSpace dataSpace(H5S_SCALAR);
        H5::DataSet dataSet = g.createDataSet(n, NativeType<T>(), dataSpace);
        dataSet.write(&v, NativeType<T>());
    }
}

template<typename T>
T H5DB::Load_impl(const H5::Group &g, const std::string &n, std::true_type)
{
    T val=T();
    if (g.nameExists(n))
    {
        H5::DataSet dataset = g.openDataSet(n);
        H5::DataType datatype = dataset.getDataType();
        H5::PredType predtype = NativeType<T>();
        if (datatype.getClass() == predtype.getClass())
        {
            H5::DataSpace s = dataset.getSpace();
            if (H5S_SCALAR == s.getSimpleExtentType())
            {
                dataset.read(&val, predtype);
            }
        }
    }

    return val;
}

template<typename T>
void H5DB::SetAttribute_impl(const H5::H5Object &o, const std::string &n, const T &v, std::true_type)
{
    if (o.attrExists(n))
    {
        o.removeAttr(n);
    }

    if (!o.attrExists(n))
    {
        H5::DataSpace attSpace(H5S_SCALAR);
        H5::Attribute att = o.createAttribute(n, NativeType<T>(), attSpace);
        att.write(NativeType<T>(), &v);
    }
}

template<typename T>
T H5DB::GetAttribute_impl(const H5::H5Object &g, const std::string &n, std::true_type)
{
    T val = T();
    if (g.attrExists(n))
    {
        H5::Attribute a = g.openAttribute(n);
        H5::PredType predtype = NativeType<T>();
        if (a.getTypeClass() == predtype.getClass())
        {
            H5::DataSpace s = a.getSpace();
            if (H5S_SCALAR == s.getSimpleExtentType())
            {
                a.read(predtype, &val);
            }
        }
    }

    return val;
}

template<typename T>
void H5DB::Save(const H5::Group &g, const std::string &n, const T &v)
{
    Save_impl(g, n, v, std::is_pod<T>());
}

template<typename T>
void H5DB::Save(const H5::Group &g, const std::string &n, const std::vector<T> &seq)
{
    if (g.nameExists(n))
    {
        H5Ldelete(g.getId(), n.data(), H5P_DEFAULT);
    }

    if (!g.nameExists(n))
    {
        int rank = 1;
        const hsize_t dims[1] = { seq.size() };
        H5::DataSpace dataSpace(rank, dims);
        H5::DataSet dataSet = g.createDataSet(n, NativeType<T>(), dataSpace);

        dataSet.write(seq.data(), NativeType<T>());
    }
}

template<typename T, std::size_t Dim>
void H5DB::Save(const H5::Group &g, const std::string &n, const std::vector<std::array<T, Dim>> &points)
{
    if (g.nameExists(n))
    {
        H5Ldelete(g.getId(), n.data(), H5P_DEFAULT);
    }

    if (!g.nameExists(n))
    {
        hsize_t dimsm[1] = { Dim };
        H5::ArrayType arrType(NativeType<T>(), 1, dimsm);

        int rank = 1;
        const hsize_t dims[1] = { points.size() };
        H5::DataSpace dataSpace(rank, dims);
        H5::DataSet dataSet = g.createDataSet(n, arrType, dataSpace);

        dataSet.write(&points.front()[0], arrType);
    }
}

template<> wxColour H5DB::Load<wxColour>(const H5::Group &g, const std::string &n);
template<> std::string H5DB::GetAttribute<std::string>(const H5::H5Object &o, const std::string &n);
template<> wxColour H5DB::GetAttribute<wxColour>(const H5::H5Object &o, const std::string &n);

template<typename T>
T H5DB::Load(const H5::Group &g, const std::string &n)
{
    return Load_impl<T>(g, n, std::is_pod<T>());
}

template<typename T>
bool H5DB::Load(const H5::Group &g, const std::string &n, std::vector<T> &seq)
{
    seq.clear();

    if (g.nameExists(n))
    {
        H5::DataSet  dataset = g.openDataSet(n);
        H5::DataType datatype = dataset.getDataType();

        const H5::DataType &dType = NativeType<T>();
        if (datatype.getClass() == dType.getClass())
        {
            H5::DataSpace s = dataset.getSpace();
            if (H5S_SIMPLE == s.getSimpleExtentType() &&
                1 == s.getSimpleExtentNdims())
            {
                hsize_t dims[1] = { 0 };
                s.getSimpleExtentDims(dims);
                if (dims[0]>0)
                {
                    seq.resize(dims[0]);
                    dataset.read(seq.data(), datatype);
                    return true;
                }
            }
        }
    }

    return false;
}

template<typename T, std::size_t Dim>
bool H5DB::Load(const H5::Group &g, const std::string &n, std::vector<std::array<T, Dim>> &points)
{
    points.clear();
    if (g.nameExists(n))
    {
        H5::DataSet dataset = g.openDataSet(n);
        H5::DataType datatype = dataset.getDataType();

        hsize_t dimsm[1] = { Dim };
        H5::ArrayType arrType(NativeType<T>(), 1, dimsm);
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
                    if (Dim == dimsm[0])
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

template<typename T>
void H5DB::SetAttribute(const H5::H5Object &o, const std::string &n, const T &v)
{
    SetAttribute_impl(o, n, v, std::is_pod<T>());
}

template<typename T>
T H5DB::GetAttribute(const H5::H5Object &o, const std::string &n)
{
    return GetAttribute_impl<T>(o, n, std::is_pod<T>());
}

template<typename T, std::size_t Dim>
void H5DB::Save(const H5::Group &g, const std::string &n, const std::array<T, Dim> &arr)
{
    if (g.nameExists(n))
    {
        H5Ldelete(g.getId(), n.data(), H5P_DEFAULT);
    }

    if (!g.nameExists(n))
    {
        hsize_t dimsm[1] = { Dim };
        H5::DataSpace dataSpace(1, dimsm);
        H5::DataSet dataSet = g.createDataSet(n, NativeType<T>(), dataSpace);

        dataSet.write(arr.data(), NativeType<T>());
    }
}

template<typename T, std::size_t Dim>
bool H5DB::Load(const H5::Group &g, const std::string &n, std::array<T, Dim> &arr)
{
    if (g.nameExists(n))
    {
        H5::DataSet dataset = g.openDataSet(n);
        H5::DataType datatype = dataset.getDataType();

        hsize_t dims[1] = { Dim };
        const H5::PredType &elemType = NativeType<T>();
        if (datatype.getClass() == elemType.getClass())
        {
            H5::DataSpace s = dataset.getSpace();
            if (H5S_SIMPLE == s.getSimpleExtentType() &&
                1 == s.getSimpleExtentNdims())
            {
                hsize_t dims[1] = { 0 };
                s.getSimpleExtentDims(dims);
                if (Dim == dims[0])
                {
                    dataset.read(arr.data(), elemType);
                    return true;
                }
            }
        }
    }

    return false;
}

#endif  // SPAM_HELPER_H5DB_H