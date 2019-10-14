#include "spamutility.h"
#include <2geom/cairo-path-sink.h>
#include <cairomm/cairomm.h>

cv::Mat SpamUtility::GetMaskFromPath(const Geom::PathVector &pv, const cv::Size &sz)
{
    int step = Cairo::ImageSurface::format_stride_for_width(Cairo::Format::FORMAT_A8, sz.width);
    std::unique_ptr<uchar[]> imgData{ new uchar[sz.height*step]() };
    cv::Mat mask(sz.height, sz.width, CV_8UC1, imgData.get(), step);
    auto imgSurf = Cairo::ImageSurface::create(mask.data, Cairo::Format::FORMAT_A8, mask.cols, mask.rows, mask.step1());
    auto cr = Cairo::Context::create(imgSurf);

    Geom::CairoPathSink cairoPathSink(cr->cobj());
    cairoPathSink.feed(pv);
    cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
    cr->fill();

    mask = mask.clone();
    return mask;
}