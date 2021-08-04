#include "endstep.h"
#include <wx/dcgraph.h>
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>

const std::vector<std::tuple<int, int>> EndStep::inPorts_s = { std::make_tuple(-1, kSPT_ANY) };
const std::vector<std::tuple<int, int>> EndStep::outPorts_s = {};

EndStep::EndStep()
    : StepBase(wxString(wxT("End")))
{
}

void EndStep::Draw(wxGCDC &dc) const
{
    dc.SetFont(*wxNORMAL_FONT);
    wxPen outLinePen(IsHighlight() ? wxColour(0xF9, 0xA6, 0x02) : wxColour(214, 219, 233), 2, wxPENSTYLE_SOLID);
    dc.SetPen(outLinePen);
    dc.SetBrush(wxNullBrush);
    dc.DrawEllipse(posRect_.GetTopLeft(), posRect_.GetSize());
    dc.SetTextForeground(IsHighlight() ? wxColour(0xF9, 0xA6, 0x02) : wxColour(214, 219, 233));
    dc.DrawText(typeName_, posRect_.GetTopLeft() + wxPoint((posRect_.GetWidth() - htSize_.GetWidth()) / 2, (posRect_.GetHeight() - htSize_.GetHeight()) / 2));
}

const int EndStep::GetInPortCount() const
{
    return static_cast<int>(inPorts_s.size());
}

const int EndStep::GetOutPortCount() const
{
    return static_cast<int>(outPorts_s.size());
}

const int EndStep::GetInPortDegree(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<0>(inPorts_s[portIndex]);
}

const int EndStep::GetOutPortDegree(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<0>(outPorts_s[portIndex]);
}

const int EndStep::GetInPortType(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<1>(inPorts_s[portIndex]);
}

const int EndStep::GetOutPortType(const int portIndex) const
{
    if (portIndex < 0 && portIndex >= GetInPortCount())
    {
        throw std::out_of_range(std::to_string(portIndex));
    }

    return std::get<1>(outPorts_s[portIndex]);
}
