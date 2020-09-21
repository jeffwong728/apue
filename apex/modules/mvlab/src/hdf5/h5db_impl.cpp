#include "precomp.hpp"
#include "h5db_impl.hpp"
#include "h5group_impl.hpp"
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

cv::Ptr<H5DB> H5DB::Open(const cv::String &HDF5Filename)
{
    try
    {
        H5::Exception::dontPrint();

        std::error_code ec;
        unsigned int flags = 0;
        std::experimental::filesystem::path p(HDF5Filename);
        if (std::experimental::filesystem::exists(p, ec))
        {
            if (std::experimental::filesystem::is_regular_file(p, ec)
                && H5::H5File::isHdf5(HDF5Filename))
            {
                flags = H5F_ACC_RDWR;
            }
            else
            {
                return makePtr<H5DBImpl>("Not a valid HDF5 file");
            }
        }
        else
        {
            flags = H5F_ACC_TRUNC;
        }

        return makePtr<H5DBImpl>(H5::H5File(HDF5Filename, flags));
    }
    catch (const H5::Exception &e)
    {
        return makePtr<H5DBImpl>(e.getCDetailMsg());
    }
}

bool H5DBImpl::Valid() const
{
    return H5::H5File::isValid(file_.getId());
}

void H5DBImpl::Close()
{
    if (H5DBImpl::Valid())
    {
        file_.close();
    }
}

cv::String H5DBImpl::GetErrorStatus() const
{
    return err_msg_;
}

cv::Ptr<H5Group> H5DBImpl::GetRoot() const
{
    return H5DBImpl::GetGroup("/");
}

cv::Ptr<H5Group> H5DBImpl::GetGroup(const cv::String &name) const
{
    try
    {
        H5::Exception::dontPrint();
        if (file_.nameExists(name))
        {
            return makePtr<H5GroupImpl>(file_.openGroup(name));
        }
        else
        {
            H5::LinkCreatPropList lcpl;
            lcpl.setCharEncoding(H5T_CSET_UTF8);
            return makePtr<H5GroupImpl>(file_.createGroup(name, lcpl));
        }
    }
    catch (const H5::Exception &e)
    {
        return makePtr<H5GroupImpl>(e.getCDetailMsg());
    }
}

}
}
