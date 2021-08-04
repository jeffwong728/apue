#include "cvtstep.h"

const std::vector<std::tuple<int, int>> CvtStep::inPorts_s = { std::make_tuple(1, kSPT_MAT) };
const std::vector<std::tuple<int, int>> CvtStep::outPorts_s = { std::make_tuple(-1, kSPT_MAT) };

CvtStep::CvtStep()
    : StepBase(wxString(wxT("Color Convert")))
{
}

const int CvtStep::GetInPortCount() const
{
    return static_cast<int>(inPorts_s.size());
}

const int CvtStep::GetOutPortCount() const
{
    return static_cast<int>(outPorts_s.size());
}

const int CvtStep::GetInPortDegree(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<0>(inPorts_s[portIndex]);
}

const int CvtStep::GetOutPortDegree(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<0>(outPorts_s[portIndex]);
}

const int CvtStep::GetInPortType(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<1>(inPorts_s[portIndex]);
}

const int CvtStep::GetOutPortType(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<1>(outPorts_s[portIndex]);
}
