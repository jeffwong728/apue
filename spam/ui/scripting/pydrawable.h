#ifndef SPAM_UI_SCRIPTING_PYDRAWABLE_H
#define SPAM_UI_SCRIPTING_PYDRAWABLE_H

#include "mvlabcaster.h"
#include <ui/projs/modelfwd.h>

struct PyDrawable
{
    WPDrawableNode wpObj;
    void SetColor(const RGBATuple &color);
    pybind11::object GetColor() const;
    std::string toString() const;
    virtual ~PyDrawable() = default;
};

#endif //SPAM_UI_SCRIPTING_PYDRAWABLE_H
