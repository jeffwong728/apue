#include "precomp.hpp"
#include "match_result_impl.hpp"
#include "pixel_template_impl.hpp"
#include <utility.hpp>
#include <region/region_array_impl.hpp>
#include <hdf5/h5group_impl.hpp>
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

const cv::String PixelTemplateImpl::type_guid_s{ "F96B93A4-3C4F-404D-A0A1-398FEF1A7245" };

cv::Ptr<PixelTemplate> PixelTemplate::GenEmpty()
{
    return makePtr<PixelTemplateImpl>();
}

cv::Ptr<PixelTemplate> PixelTemplate::GenTemplate(cv::InputArray img, const cv::Ptr<Region> &rgn, const cv::Ptr<Dict> &opts)
{
    cv::Ptr<PixelTemplate> tmpl = PixelTemplate::GenEmpty();
    tmpl->Create(img, rgn, opts);
    return tmpl;
}

cv::Ptr<PixelTemplate> PixelTemplate::Load(const cv::String &fileName, const cv::Ptr<Dict> &opts)
{
    cv::Ptr<PixelTemplateImpl> tmpl = makePtr<PixelTemplateImpl>();
    tmpl->Load(fileName, opts);
    return tmpl;
}

PixelTemplateImpl::PixelTemplateImpl(const std::string &bytes)
{
    try
    {
        std::istringstream iss(bytes);
        boost::iostreams::filtering_istream fin;
        fin.push(boost::iostreams::lzma_decompressor());
        fin.push(iss);

        boost::archive::text_iarchive ia(fin);
        ia >> boost::serialization::make_nvp("pixel_template", *this);
        if (tmpl_rgn_)
        {
            tmpl_cont_ = tmpl_rgn_->GetContour()->Simplify(1.5f);
        }     
    }
    catch (const std::exception &e)
    {
        err_msg_ = e.what();
    }
}

bool PixelTemplateImpl::Empty() const
{
    return pyramid_tmpl_datas_.empty();
}

cv::String PixelTemplateImpl::GetErrorStatus() const
{
    return err_msg_;
}

int PixelTemplateImpl::GetPyramidLevel() const
{
    return pyramid_level_;
}

int PixelTemplateImpl::Create(cv::InputArray img, const cv::Ptr<Region> &rgn, const cv::Ptr<Dict> &opts)
{
    cv::Mat imgMat = img.getMat();
    if (imgMat.empty())
    {
        return MLR_IMAGE_EMPTY;
    }

    int dph = imgMat.depth();
    int cnl = imgMat.channels();
    if (CV_8U != dph || 1 != cnl || 2 != imgMat.dims)
    {
        return MLR_IMAGE_FORMAT_ERROR;
    }

    if (rgn.empty() || rgn->Empty())
    {
        return MLR_TEMPLATE_EMPTY_TEMPL_REGION;
    }

    PixelTmplCreateData ptcd;
    ptcd.img = imgMat;
    ptcd.rgn = rgn;
    ptcd.angleStart = -30;
    ptcd.angleExtent = 60;
    ptcd.pyramidLevel = 3;
    GetTemplateOptions(opts, ptcd);

    return CreateTemplate(ptcd);
}

cv::Ptr<MatchResult> PixelTemplateImpl::Search(cv::InputArray img, const cv::Ptr<Dict> &opts) const
{
    err_msg_.resize(0);
    cv::Ptr<MatchResultImpl> mr = cv::makePtr<MatchResultImpl>(const_cast<PixelTemplateImpl *>(this)->shared_from_this());
    if (!mr)
    {
        err_msg_ = "memory error";
        return mr;
    }

    cv::Mat mat = img.getMat();
    if (mat.empty() || 2 != mat.dims || 1 != mat.channels() || CV_8U != mat.depth())
    {
        err_msg_ = "image empty or image format error";
        mr->SetErrorMessage(err_msg_);
        return mr;
    }

    cv::Point2f pos;
    float angle = 0.f;
    float score = 0.f;
    int r = MLR_FAILURE;
    if (match_mode_ == "ncc")
    {
        const float minScore = opts->GetReal32("MinScore", 0.8f);
        r = const_cast<PixelTmpl *>(static_cast<const PixelTmpl *>(this))->matchNCCTemplate(mat, minScore, pos, angle, score);
    }
    else
    {
        const int sad = opts->GetInt("MaxSad", 10);
        r = const_cast<PixelTmpl *>(static_cast<const PixelTmpl *>(this))->matchPixelTemplate(mat, sad, pos, angle, score);
    }

    if (MLR_SUCCESS != r)
    {
        err_msg_ = "match error";
        mr->SetErrorMessage(err_msg_);
        return mr;
    }

    mr->AddInstance(score, pos, angle);
    return mr;
}

int PixelTemplateImpl::Draw(cv::InputOutputArray img, const cv::Ptr<Dict> &opts) const
{
    if (opts)
    {
        cv::Vec4b colorTab[] =
        {
            cv::Vec4b(255, 0,   0,   255),
            cv::Vec4b(0, 0,   0,   255),
            cv::Vec4b(0,   255, 0,   255),
            cv::Vec4b(255, 64, 0, 255),
            cv::Vec4b(64, 255, 0, 255),
            cv::Vec4b(100, 100, 0, 255),
            cv::Vec4b(255, 255, 0,   255),
            cv::Vec4b(255, 128, 0, 255),
            cv::Vec4b(128, 255, 0,   255)
        };
    }

    return MLR_SUCCESS;
}

int PixelTemplateImpl::Save(const cv::String &fileName, const cv::Ptr<Dict> &opts) const
{
    return WriteToFile<PixelTemplateImpl>(*this, "pixel_template", fileName, opts, err_msg_);
}

int PixelTemplateImpl::Load(const cv::String &fileName, const cv::Ptr<Dict> &opts)
{
    destroyData();
    int r = LoadFromFile<PixelTemplateImpl>(*this, "pixel_template", fileName, opts, err_msg_);
    if (MLR_SUCCESS == r)
    {
        tmpl_cont_ = tmpl_rgn_->GetContour()->Simplify(1.5f);
    }
    return r;
}

int PixelTemplateImpl::Serialize(const cv::String &name, H5Group *g) const
{
    err_msg_.resize(0);
    std::ostringstream bytes;
    H5GroupImpl *group = dynamic_cast<H5GroupImpl *>(g);
    if (!group || !group->Valid())
    {
        return MLR_H5DB_INVALID;
    }

    try
    {
        {
            boost::iostreams::filtering_ostream fout;
            fout.push(boost::iostreams::lzma_compressor());
            fout.push(bytes);
            boost::archive::text_oarchive oa(fout);
            oa << boost::serialization::make_nvp("pixel_template", *this);
        }

        H5::DataSet dataSet;
        int r = group->SetDataSet(name, bytes.str(), dataSet);
        if (MLR_SUCCESS != r)
        {
            return r;
        }

        cv::InputArray arr(cfs_);
        group->SetAttribute(dataSet, cv::String("TypeGUID"), PixelTemplateImpl::TypeGUID());
        group->SetAttribute(dataSet, cv::String("Version"), 0);
        group->SetAttribute(dataSet, cv::String("PyramidLevel"), pyramid_level_);
        group->SetAttribute(dataSet, cv::String("CentreOfReferance"), arr.getMat());
    }
    catch (const std::exception &e)
    {
        err_msg_ = e.what();
        group->SetErrorMessage(err_msg_);
        return MLR_IO_STREAM_EXCEPTION;
    }

    return MLR_SUCCESS;
}

void PixelTemplateImpl::GetTemplateOptions(const cv::Ptr<Dict> &opts, PixelTmplCreateData &ptcd)
{
    if (opts)
    {
        ptcd.roi            = opts->GetRegion("ROI");
        ptcd.angleStart     = opts->GetInt("AngleStart",   ptcd.angleStart);
        ptcd.angleExtent    = opts->GetInt("AngleExtent",  ptcd.angleExtent);
        ptcd.pyramidLevel   = opts->GetInt("PyramidLevel", ptcd.pyramidLevel);
        ptcd.matchMode      = opts->GetString("MatchMode");
    }
}

}
}
