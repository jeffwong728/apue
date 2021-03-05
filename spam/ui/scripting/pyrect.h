#ifndef SPAM_UI_SCRIPTING_PYRECT_H
#define SPAM_UI_SCRIPTING_PYRECT_H

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <ui/projs/modelfwd.h>

struct PyRect
{
    double x, y, w, h;
    WPRectNode wpRect;
    PyRect();
    void SetX(const double x_) { x = x_; }
    void SetY(const double y_) { y = y_; }
    void SetWidth(const double w_);
    void SetHeight(const double h_);
    std::string toString() const;
    double GetX() const { return x; }
    double GetY() const { return y; }
    double GetWidth() const { return w; }
    double GetHeight() const { return h; }
    void Translate(const double delta_x, const double delta_y);
};

#endif //SPAM_UI_SCRIPTING_PYRECT_H
