#ifndef SPAM_UI_SCRIPTING_PYSTATION_H
#define SPAM_UI_SCRIPTING_PYSTATION_H

#include "mvlabcaster.h"
#include <ui/projs/modelfwd.h>
#include <cstdint>

struct PyStation
{
    std::string GetName() const;
    pybind11::object NewRect(const double center_x, const double center_y, const double width, const double height);
    pybind11::object NewEllipse(const double center_x, const double center_y, const double width, const double height);
    pybind11::object FindEntity(const std::string &name);
    pybind11::object GetAllEntities();
    pybind11::object GetSelected();
    pybind11::object FuncTest(pybind11::args args, pybind11::kwargs kwargs);
    void DispObj(const cv::Ptr<cv::mvlab::Region> &obj);
    void DispObj(const cv::Ptr<cv::mvlab::Contour> &obj);
    void DispCircle(const PyPoint2 &center, const double radius);
    void DispArc(const PyPoint2 &center, const double radius,const PyRange &angle_range, const std::string &specification=std::string("clockwise"));
    void DispArrow(const PyPoint2 &start_point, const PyPoint2 &end_point, const double head_size);
    void DispCross(const PyPoint2 &center, const double size, const double angle);
    void DispEllipse(const PyPoint2 &center, const double phi, const PyPoint2 &radii);
    void DispLine(const PyPoint2 &start_point, const PyPoint2 &end_point);
    void DispPolygon(const std::vector<PyPoint2> &vertexes);
    void DispPolyline(const std::vector<PyPoint2> &points);
    void DispRectangle1(const PyPoint2 &point1, const PyPoint2 &point2);
    void DispRectangle2(const PyPoint2 &center, const double phi, const PyPoint2 &lengths);
    void EraseObj(const cv::Ptr<cv::mvlab::Region> &obj);
    void EraseBoxArea(const PyPoint2 &point1, const PyPoint2 &point2);
    void EraseFullArea();
    void SetDraw(const std::string &mode);
    pybind11::object GetDraw() const;
    void SetColor(const std::string &color);
    void SetColor(const uint8_t red, const uint8_t green, const uint8_t blue);
    void SetColor(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha);
    void SetColor(const std::vector<std::string> &colors);
    void SetColor(const std::vector<RGBTuple> &colors);
    void SetColor(const std::vector<RGBATuple> &colors);
    void SetColored(const int number_of_colors=12);
    pybind11::object GetColor() const;
    void SetImage(const cv::Mat &image);
    pybind11::object GetImage() const;
    void SetLineWidth(const double width);
    pybind11::object GetLineWidth() const;
    WPStationNode wpPyStation;
};

#endif //SPAM_UI_SCRIPTING_PYSTATION_H
