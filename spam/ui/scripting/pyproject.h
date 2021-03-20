#ifndef SPAM_UI_SCRIPTING_PYPROJECT_H
#define SPAM_UI_SCRIPTING_PYPROJECT_H

#include "mvlabcaster.h"
#include "pystation.h"
#include <ui/projs/modelfwd.h>
#include <cstdint>

struct PyProject
{
    std::string GetName() const;
    WPProjNode wpPyProject;
    std::vector<PyStation> GetAllStations() const;
};

#endif //SPAM_UI_SCRIPTING_PYPROJECT_H
