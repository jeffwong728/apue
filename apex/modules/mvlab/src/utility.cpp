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

}
}
