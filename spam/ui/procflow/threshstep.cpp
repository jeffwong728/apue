#include "threshstep.h"

const std::vector<std::tuple<int, int>> ThreshStep::inPorts_s = { std::make_tuple(1, kSPT_MAT) };
const std::vector<std::tuple<int, int>> ThreshStep::outPorts_s = { std::make_tuple(-1, kSPT_REGION) };

ThreshStep::ThreshStep()
    : StepBase(wxString(wxT("Threshold")))
{
}

const int ThreshStep::GetInPortCount() const
{
    return static_cast<int>(inPorts_s.size());
}

const int ThreshStep::GetOutPortCount() const
{
    return static_cast<int>(outPorts_s.size());
}

const int ThreshStep::GetInPortDegree(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<0>(inPorts_s[portIndex]);
}

const int ThreshStep::GetOutPortDegree(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<0>(outPorts_s[portIndex]);
}

const int ThreshStep::GetInPortType(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<1>(inPorts_s[portIndex]);
}

const int ThreshStep::GetOutPortType(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<1>(outPorts_s[portIndex]);
}
