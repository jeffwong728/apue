#ifndef SPAM_UI_SCRIPTING_PYSTATION_H
#define SPAM_UI_SCRIPTING_PYSTATION_H

#include "mvlabcaster.h"
#include <ui/projs/modelfwd.h>
#include <cstdint>

struct PyStation
{
    std::string GetName() const;
    pybind11::object NewRect(const double center_x, const double center_y, const double width, const double height);
    pybind11::object FuncTest(pybind11::args args, pybind11::kwargs kwargs);
    void DispRegion(const cv::Ptr<cv::mvlab::Region> &region);
    void EraseRegion(const cv::Ptr<cv::mvlab::Region> &region);
    void DispObj(const cv::Ptr<cv::mvlab::Region> &obj);
    void DispObj(const cv::Ptr<cv::mvlab::Contour> &obj);
    void SetDraw(const std::string &mode);
    std::string GetDraw() const;
    void SetColor(const std::string &color);
    void SetColor(const RGBTuple &color);
    void SetColor(const RGBATuple &color);
    void SetColor(const std::vector<std::string> &colors);
    void SetColor(const std::vector<RGBTuple> &colors);
    void SetColor(const std::vector<RGBATuple> &colors);
    void SetColored(const int number_of_colors=12);
    WPStationNode wpPyStation;
};

#endif //SPAM_UI_SCRIPTING_PYSTATION_H
