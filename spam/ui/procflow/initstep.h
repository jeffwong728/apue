#ifndef SPAM_UI_PROCFLOW_INIT_STEP_H
#define SPAM_UI_PROCFLOW_INIT_STEP_H
#include "stepbase.h"

class InitStep : public StepBase
{
public:
    InitStep();

private:
    std::string stationUUID_;
};
#endif //SPAM_UI_PROCFLOW_INIT_STEP_H
