#include "precomp.hpp"
#include "utility.hpp"

namespace cv {
namespace mvlab {

std::vector<double> Util::GetDashesPattern(const int bls, const double lineWidth)
{
    switch (bls)
    {
    case BOUNDARY_LINE_DASH: return { 3 * lineWidth, 3 * lineWidth };
    case BOUNDARY_LINE_DOT: return { 1 * lineWidth, 1 * lineWidth };
    case BOUNDARY_LINE_DASHDOT: return { 3 * lineWidth, 1 * lineWidth, 1 * lineWidth, 1 * lineWidth };
    case BOUNDARY_LINE_DASHDOTDOT: return { 3 * lineWidth, 1 * lineWidth, 1 * lineWidth, 1 * lineWidth, 1 * lineWidth, 1 * lineWidth };
    default: return {};
    }
}

cv::Mat Util::PathToMask(const Geom::PathVector &pv, const cv::Size &sz)
{
    int step = Cairo::ImageSurface::format_stride_for_width(Cairo::Format::FORMAT_A8, sz.width);
    std::unique_ptr<uchar[]> imgData{ new uchar[sz.height*step]() };
    cv::Mat mask(sz.height, sz.width, CV_8UC1, imgData.get(), step);
    auto imgSurf = Cairo::ImageSurface::create(mask.data, Cairo::Format::FORMAT_A8, mask.cols, mask.rows, static_cast<int>(mask.step1()));
    auto cr = Cairo::Context::create(imgSurf);
    cr->translate(0.5, 0.5);

    Geom::CairoPathSink cairoPathSink(cr->cobj());
    cairoPathSink.feed(pv);
    cr->set_line_width(0.);
    cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
    cr->fill();

    mask = mask.clone();
    return mask;
}

cv::Mat Util::PathToMask(const Geom::PathVector &pv, const cv::Size &sz, UScalableUCharSequence &buf)
{
    int step = Cairo::ImageSurface::format_stride_for_width(Cairo::Format::FORMAT_A8, sz.width);
    buf.resize(sz.height*step);
    std::memset(buf.data(), 0, buf.size()*sizeof(UScalableUCharSequence::value_type));

    cv::Mat mask(sz.height, sz.width, CV_8UC1, buf.data(), step);
    auto imgSurf = Cairo::ImageSurface::create(mask.data, Cairo::Format::FORMAT_A8, mask.cols, mask.rows, static_cast<int>(mask.step1()));
    auto cr = Cairo::Context::create(imgSurf);
    cr->translate(0.5, 0.5);

    Geom::CairoPathSink cairoPathSink(cr->cobj());
    cairoPathSink.feed(pv);
    cr->set_line_width(0.);
    cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
    cr->fill();

    return mask;
}

int Util::CheckSaveParameters(const cv::String &fileName, const cv::Ptr<Dict> &opts, cv::String &errMsg)
{
    errMsg.resize(0);
    std::experimental::filesystem::path filePath(fileName);
    if (filePath.empty())
    {
        errMsg = "file name empty";
        return MLR_PARAMETER_ERROR_FILE_PATH;
    }

    cv::String policy = opts ? opts->GetString("Policy") : "backup";
    if (std::experimental::filesystem::exists(filePath)) {
        if (std::experimental::filesystem::is_directory(filePath)) {
            errMsg = "existing directory";
            return MLR_PARAMETER_ERROR_EXISTING_DIRECTORY;
        }
        else {
            if (policy == "raise_error") {
                errMsg = "existing file";
                return MLR_PARAMETER_ERROR_EXISTING_FILE;
            }
            else if (policy == "overwrite") {
                std::experimental::filesystem::remove(filePath);
            }
            else {
                std::experimental::filesystem::path backFilePath;
                do {
                    auto t = std::time(nullptr);
                    auto tm = *std::localtime(&t);
                    std::ostringstream oss;
                    oss << std::put_time(&tm, "_%d-%m-%Y_%H-%M-%S_backup");
                    backFilePath = filePath;
                    backFilePath.replace_filename(filePath.stem().concat(oss.str())).concat(filePath.extension().string());
                } while (std::experimental::filesystem::exists(backFilePath));

                std::experimental::filesystem::copy_file(filePath, backFilePath);
                std::experimental::filesystem::remove(filePath);
            }
        }
    }

    return MLR_SUCCESS;
}

int Util::CheckLoadParameters(const cv::String &fileName, const cv::Ptr<Dict> &opts, cv::String &formatHint, cv::String &errMsg)
{
    errMsg.resize(0);
    std::experimental::filesystem::path filePath(fileName);
    if (!std::experimental::filesystem::exists(filePath) || !std::experimental::filesystem::is_regular_file(filePath))
    {
        errMsg = "file path error";
        return MLR_PARAMETER_ERROR_FILE_PATH;
    }

    cv::String sigXML("<?xml");
    cv::String sigTxt("22 serialization::archive");
    cv::String sigFirst(32, '\0');

    std::ifstream fin(fileName, std::ofstream::in);
    if (fin.fail())
    {
        errMsg = "open file error";
        return MLR_FILESYSTEM_EXCEPTION;
    }

    int cChars = 0;
    while (fin >> std::noskipws >> sigFirst[cChars] && cChars < 32)
    {
        cChars += 1;
    }

    fin.close();

    if (sigFirst.rfind(sigXML, 0) == 0)
    {
        formatHint = "xml";
    }
    else if (sigFirst.rfind(sigTxt, 0) == 0)
    {
        formatHint = "text";
    }
    else
    {
        formatHint = opts ? opts->GetString("FormatHint") : "binary";
    }

    return MLR_SUCCESS;
}

int Util::CheckCompressLoadParameters(const cv::String &fileName, const cv::Ptr<Dict> &opts, cv::String &formatHint, cv::String &errMsg)
{
    errMsg.resize(0);
    std::experimental::filesystem::path filePath(fileName);
    if (!std::experimental::filesystem::exists(filePath) || !std::experimental::filesystem::is_regular_file(filePath))
    {
        errMsg = "file path error";
        return MLR_PARAMETER_ERROR_FILE_PATH;
    }

    cv::String sigXML("<?xml");
    cv::String sigTxt("22 serialization::archive");
    cv::String sigFirst(32, '\0');

    std::ifstream fin(fileName, std::ofstream::in | std::ofstream::binary);
    if (fin.fail())
    {
        errMsg = "open file error";
        return MLR_FILESYSTEM_EXCEPTION;
    }

    {
        boost::iostreams::filtering_istream fiin;
        fiin.push(boost::iostreams::lzma_decompressor());
        fiin.push(fin);

        int cChars = 0;
        while (fiin >> std::noskipws >> sigFirst[cChars] && cChars < 32)
        {
            cChars += 1;
        }
    }

    fin.close();

    if (sigFirst.rfind(sigXML, 0) == 0)
    {
        formatHint = "xml";
    }
    else if (sigFirst.rfind(sigTxt, 0) == 0)
    {
        formatHint = "text";
    }
    else
    {
        formatHint = opts ? opts->GetString("FormatHint") : "binary";
    }

    return MLR_SUCCESS;
}

class MemZeroer
{
public:
    MemZeroer(char *const dest)
        : dest_(dest)
    {}

    void operator()(const tbb::blocked_range<int>& br) const
    {
        vcl::Vec32uc val(0);
        constexpr int vectorsize = 32;
        char *dstEnd = dest_ + br.end() * vectorsize;
        for (char *dst = dest_ + br.begin() * vectorsize; dst != dstEnd; dst += vectorsize)
        {
            val.store(dst);
        }
    }

private:
    char *const dest_;
};

void Util::SIMDZeroMemory(void *dest, const int sz)
{
    char *const dst = (char *)dest;
    constexpr int vectorsize = 32;
    if (sz > vectorsize * 256 * 16)
    {
        const int regularpart = sz & (-vectorsize);
        tbb::parallel_for(tbb::blocked_range<int>(0, regularpart / vectorsize), MemZeroer(dst));
        std::memset(dst + regularpart, 0, sz - regularpart);
    }
    else
    {
        std::memset(dst, 0, sz);
    }
}

}
}
