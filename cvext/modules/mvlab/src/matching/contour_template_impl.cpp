#include "precomp.hpp"
#include "match_result_impl.hpp"
#include "contour_template_impl.hpp"
#include <utility.hpp>
#include <region/region_array_impl.hpp>
#include <hdf5/h5group_impl.hpp>
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

const cv::String ContourTemplateImpl::type_guid_s{ "638A07EC-5E7E-4BEB-A559-8256720C0E7F" };

ContourTemplateImpl::ContourTemplateImpl(const std::string &bytes) 
{
    try
    {
        std::istringstream iss(bytes);
        boost::iostreams::filtering_istream fin;
        fin.push(boost::iostreams::lzma_decompressor());
        fin.push(iss);

        boost::archive::text_iarchive ia(fin);
        ia >> boost::serialization::make_nvp("contour_template", *this);
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

bool ContourTemplateImpl::Empty() const
{
    return pyramid_tmpl_datas_.empty();
}

cv::String ContourTemplateImpl::GetErrorStatus() const
{
    return err_msg_;
}

int ContourTemplateImpl::GetPyramidLevel() const
{
    return pyramid_level_;
}

int ContourTemplateImpl::Create(cv::InputArray img, const cv::Ptr<Region> &rgn, const cv::Ptr<Dict> &opts)
{
    cv::Mat imgMat = img.getMat();
    if (imgMat.empty())
    {
        return MLR_IMAGE_EMPTY;
    }

    int dph = imgMat.depth();
    int cnl = imgMat.channels();
    if (CV_8U != dph || 1 != cnl)
    {
        return MLR_IMAGE_FORMAT_ERROR;
    }

    if (rgn.empty() || rgn->Empty())
    {
        return MLR_TEMPLATE_EMPTY_TEMPL_REGION;
    }

    ShapeTmplCreateData stcd;
    stcd.img = imgMat;
    stcd.rgn = rgn;
    stcd.angleStart = -30;
    stcd.angleExtent = 60;
    stcd.pyramidLevel = 3;
    stcd.lowContrast  = 10;
    stcd.highContrast = 30;
    GetTemplateOptions(opts, stcd);

    return CreateTemplate(stcd);
}

cv::Ptr<MatchResult> ContourTemplateImpl::Search(cv::InputArray img, const cv::Ptr<Dict> &opts) const
{
    err_msg_.resize(0);
    cv::Ptr<MatchResultImpl> mr = cv::makePtr<MatchResultImpl>(const_cast<ContourTemplateImpl *>(this)->shared_from_this());
    if (!mr)
    {
        err_msg_ = "memory error";
        return mr;
    }

    cv::Mat mat = img.getMat();
    if (mat.empty() || 2 != mat.dims || 1 != mat.channels() || CV_8U != mat.depth())
    {
        err_msg_ = "image empty or format error";
        mr->SetErrorMessage(err_msg_);
        return mr;
    }

    ShapeTmplMatchOption stmo{0.8f, 0.5f, 10, false, nullptr};
    if (opts)
    {
        stmo.minScore    = opts->GetReal32("MinScore", stmo.minScore);
        stmo.greediness  = opts->GetReal32("Greediness", stmo.greediness);
        stmo.minContrast = opts->GetInt("MinContrast", stmo.minContrast);
        stmo.touchBorder = opts->GetInt("TouchBorder", stmo.touchBorder);
        stmo.searchRgn   = opts->GetRegion("SearchRegion");
    }

    cv::Point2f pos;
    float angle = 0.f;
    float score = 0.f;
    int r = const_cast<ShapeTemplate *>(static_cast<const ShapeTemplate *>(this))->matchShapeTemplate(mat, stmo, pos, angle, score);
    if (MLR_SUCCESS != r)
    {
        err_msg_ = "match error";
        mr->SetErrorMessage(err_msg_);
        return mr;
    }

    mr->AddInstance(score, pos, angle);
    return mr;
}

int ContourTemplateImpl::Draw(cv::InputOutputArray img, const cv::Ptr<Dict> &opts) const
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

        cv::String dfp = opts->GetString("DebugFullPath");
        boost::filesystem::path dDir(dfp);
        if (boost::filesystem::exists(dDir) && boost::filesystem::is_directory(dDir))
        {
            for (int l = 0; l < static_cast<int>(pyramid_tmpl_datas_.size()); ++l)
            {
                const LayerShapeData &lsd = pyramid_tmpl_datas_[l];
                const auto &tmplDatas = lsd.tmplDatas;
                for (int t = 0; t < static_cast<int>(tmplDatas.size()); ++t)
                {
                    const ShapeTemplData &std = tmplDatas[t];
                    const int width = std.maxPoint.x - std.minPoint.x + 30;
                    const int height = std.maxPoint.y - std.minPoint.y + 30;
                    cv::Mat tmplMat(height, width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

                    int i = 0;
                    for (const cv::Point &pt : std.edgeLocs)
                    {
                        int label = 0;
                        const int partIdx = i / 8;
                        if (partIdx < static_cast<int>(std.clusters.size()))
                        {
                            label = std.clusters[partIdx].label;
                        }

                        tmplMat.at<cv::Vec4b>(pt - std.minPoint + cv::Point(15, 15)) = label ? colorTab[partIdx % (sizeof(colorTab) / sizeof(colorTab[0]))] : cv::Vec4b(0, 0, 255, 255);
                        i += 1;
                    }

                    std::string fileName = std::string("tmpl_layer_") + std::to_string(l) + std::string("_number_") + std::to_string(t) + ".png";
                    boost::filesystem::path tmplFull = dDir;
                    tmplFull.append(fileName);
                    cv::imwrite(cv::String(tmplFull.string().c_str()), tmplMat);
                }
            }
        }
    }

    return MLR_SUCCESS;
}

int ContourTemplateImpl::Save(const cv::String &fileName, const cv::Ptr<Dict> &opts) const
{
    return WriteToFile<ContourTemplateImpl>(*this, "contour_template", fileName, opts, err_msg_);
}

int ContourTemplateImpl::Load(const cv::String &fileName, const cv::Ptr<Dict> &opts)
{
    destroyData();
    int r = LoadFromFile<ContourTemplateImpl>(*this, "contour_template", fileName, opts, err_msg_);
    if (MLR_SUCCESS == r && tmpl_rgn_)
    {
        tmpl_cont_ = tmpl_rgn_->GetContour()->Simplify(1.5f);
    }

    return r;
}

int ContourTemplateImpl::Serialize(const cv::String &name, H5Group *g) const
{
    err_msg_.resize(0);
    std::ostringstream bytes;
    H5GroupImpl *group = dynamic_cast<H5GroupImpl *>(g);
    if (!group || !group->Valid())
    {
        err_msg_ = "invalid database";
        return MLR_H5DB_INVALID;
    }

    try
    {
        {
            boost::iostreams::filtering_ostream fout;
            fout.push(boost::iostreams::lzma_compressor());
            fout.push(bytes);
            boost::archive::text_oarchive oa(fout);
            oa << boost::serialization::make_nvp("contour_template", *this);
        }

        H5::DataSet dataSet;
        int r = group->SetDataSet(name, bytes.str(), dataSet);
        if (MLR_SUCCESS != r)
        {
            err_msg_ = "save database error";
            return r;
        }

        cv::InputArray arr(cfs_);
        group->SetAttribute(dataSet, cv::String("TypeGUID"),            ContourTemplateImpl::TypeGUID());
        group->SetAttribute(dataSet, cv::String("Version"),             0);
        group->SetAttribute(dataSet, cv::String("PyramidLevel"),        pyramid_level_);
        group->SetAttribute(dataSet, cv::String("AngleStart"),          angle_start_);
        group->SetAttribute(dataSet, cv::String("AngleExtent"),         angle_extent_);
        group->SetAttribute(dataSet, cv::String("LowContrast"),         low_contrast_);
        group->SetAttribute(dataSet, cv::String("HighContrast"),        high_contrast_);
        group->SetAttribute(dataSet, cv::String("CentreOfReferance"),   arr.getMat());
    }
    catch (const std::exception &e)
    {
        err_msg_ = e.what();
        return MLR_IO_STREAM_EXCEPTION;
    }

    return MLR_SUCCESS;
}

void ContourTemplateImpl::GetTemplateOptions(const cv::Ptr<Dict> &opts, ShapeTmplCreateData &stcd)
{
    if (opts)
    {
        stcd.roi            = opts->GetRegion("ROI");
        stcd.angleStart     = opts->GetInt("AngleStart",   stcd.angleStart);
        stcd.angleExtent    = opts->GetInt("AngleExtent",  stcd.angleExtent);
        stcd.pyramidLevel   = opts->GetInt("PyramidLevel", stcd.pyramidLevel);
        stcd.lowContrast    = opts->GetInt("LowContrast",  stcd.lowContrast);
        stcd.highContrast   = opts->GetInt("HighContrast", stcd.highContrast);
    }
}

}
}
