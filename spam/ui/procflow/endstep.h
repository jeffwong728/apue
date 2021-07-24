#ifndef SPAM_UI_PROCFLOW_END_STEP_H
#define SPAM_UI_PROCFLOW_END_STEP_H
#include "stepbase.h"

class EndStep : public StepBase
{
public:
    EndStep();

public:
    void Draw(wxGCDC &dc) const wxOVERRIDE;
};
#endif //SPAM_UI_PROCFLOW_END_STEP_H
