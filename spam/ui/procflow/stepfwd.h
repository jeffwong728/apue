#ifndef SPAM_UI_PROCFLOW_STEP_FORWARD_H
#define SPAM_UI_PROCFLOW_STEP_FORWARD_H
#include <string>
#include <memory>
#include <vector>

class StepBase;
class InitStep;
class EndStep;

typedef std::shared_ptr<StepBase>               SPStepBase;
typedef std::shared_ptr<InitStep>               SPInitStep;
typedef std::shared_ptr<EndStep>                SPEndStep;
typedef std::weak_ptr<StepBase>                 WPStepBase;

#endif //SPAM_UI_PROCFLOW_STEP_FORWARD_H
