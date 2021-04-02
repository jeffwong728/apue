#ifndef SPAM_UI_SCRIPTING_PYDRAWABLE_H
#define SPAM_UI_SCRIPTING_PYDRAWABLE_H

#include "pyentity.h"
#include <ui/projs/modelfwd.h>

struct PyDrawable : public PyEntity
{
    void SetColor(const RGBATuple &color);
    pybind11::object GetColor() const;
    void SetFillColor(const RGBATuple &color);
    pybind11::object GetFillColor() const;
    void SetLineWidth(const double width);
    pybind11::object GetLineWidth() const;
    pybind11::object ToRegion() const;
    std::string toString() const;
    virtual ~PyDrawable() = default;
};

#endif //SPAM_UI_SCRIPTING_PYDRAWABLE_H
