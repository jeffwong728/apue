#ifndef SPAM_UI_SCRIPTING_PYFIXED_H
#define SPAM_UI_SCRIPTING_PYFIXED_H

#include "pydrawable.h"
#include <ui/projs/modelfwd.h>

struct PyFixed : public PyDrawable
{
    PyFixed();
    bool SetFeatureImpl(const std::string &feature);
    bool ClearFeatureImpl(const std::string &feature);
    void SetFeature(const std::string &feature);
    void SetFeature(const std::vector<std::string> &features);
    void ClearFeature(const std::string &feature);
    void ClearFeature(const std::vector<std::string> &features);
    void ClearFeature();
    std::string toString() const;
};

#endif //SPAM_UI_SCRIPTING_PYFIXED_H
