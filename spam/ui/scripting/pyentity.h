#ifndef SPAM_UI_SCRIPTING_PYENTITY_H
#define SPAM_UI_SCRIPTING_PYENTITY_H

#include "mvlabcaster.h"
#include <ui/projs/modelfwd.h>

struct PyEntity
{
    WPModelNode wpObj;
    void SetName(const std::string &name);
    pybind11::object GetName() const;
    std::string toString() const;
    virtual ~PyEntity() = default;
};

#endif //SPAM_UI_SCRIPTING_PYENTITY_H
