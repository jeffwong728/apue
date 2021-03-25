#ifndef SPAM_UI_SCRIPTING_PYELLIPSE_H
#define SPAM_UI_SCRIPTING_PYELLIPSE_H

#include "pydrawable.h"
#include <ui/projs/modelfwd.h>

struct PyEllipse : public PyDrawable
{
    PyEllipse();
    void SetX(const double x);
    void SetY(const double y);
    std::string toString() const;
    double GetX() const;
    double GetY() const;
    double GetWidth() const;
    double GetHeight() const;
    double GetStartAngle() const;
    double GetEndAngle() const;
    void Translate(const double delta_x, const double delta_y);
    void Rotate(const double angle, const bool angle_as_degree=true);
};

#endif //SPAM_UI_SCRIPTING_PYELLIPSE_H
