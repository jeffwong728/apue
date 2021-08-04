#ifndef SPAM_UI_PROCFLOW_END_STEP_H
#define SPAM_UI_PROCFLOW_END_STEP_H
#include "stepbase.h"

class EndStep : public StepBase
{
public:
    EndStep();

public:
    void Draw(wxGCDC &dc) const wxOVERRIDE;

public:
    const int GetInPortCount() const wxOVERRIDE;
    const int GetOutPortCount() const wxOVERRIDE;
    const int GetInPortDegree(const int portIndex) const wxOVERRIDE;
    const int GetOutPortDegree(const int portIndex) const wxOVERRIDE;
    const int GetInPortType(const int portIndex) const wxOVERRIDE;
    const int GetOutPortType(const int portIndex) const wxOVERRIDE;

private:
    static const std::vector<std::tuple<int, int>> inPorts_s;
    static const std::vector<std::tuple<int, int>> outPorts_s;
};
#endif //SPAM_UI_PROCFLOW_END_STEP_H
