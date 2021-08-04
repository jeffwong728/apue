#include "initstep.h"
#include <wx/dcgraph.h>
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>

const std::vector<std::tuple<int, int>> InitStep::inPorts_s = {};
const std::vector<std::tuple<int, int>> InitStep::outPorts_s = { std::make_tuple(-1, kSPT_MAT) };

InitStep::InitStep()
    : StepBase(wxString(wxT("Start")))
{
}

void InitStep::DrawInternal(wxGCDC &dc) const
{
    dc.DrawRoundedRectangle(posRect_, htSize_.GetHeight());
}

const int InitStep::GetInPortCount() const
{
    return static_cast<int>(inPorts_s.size());
}

const int InitStep::GetOutPortCount() const
{
    return static_cast<int>(outPorts_s.size());
}

const int InitStep::GetInPortDegree(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<0>(inPorts_s[portIndex]);
}

const int InitStep::GetOutPortDegree(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<0>(outPorts_s[portIndex]);
}

const int InitStep::GetInPortType(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<1>(inPorts_s[portIndex]);
}

const int InitStep::GetOutPortType(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<1>(outPorts_s[portIndex]);
}
