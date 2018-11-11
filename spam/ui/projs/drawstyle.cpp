#include "drawstyle.h"
#include <wx/gdicmn.h>

wxIMPLEMENT_DYNAMIC_CLASS(DrawStyle, wxObject)
IMPLEMENT_VARIANT_OBJECT(DrawStyle)
DrawStyle::DrawStyle()
    : strokeColor_(*wxCYAN)
    , fillColor_(wxTransparentColour)
    , strokeWidth_(1)
{
}

DrawStyle::DrawStyle(const DrawStyle& ds)
    : strokeColor_(ds.strokeColor_)
    , fillColor_(ds.fillColor_)
    , strokeWidth_(ds.strokeWidth_)
{
}

DrawStyle::~DrawStyle()
{
}

DrawStyle &DrawStyle::operator=(const DrawStyle& ds)
{
    if (this != &ds)
    {
        strokeColor_ = ds.strokeColor_;
        fillColor_   = ds.fillColor_;
        strokeWidth_ = ds.strokeWidth_;
    }

    return *this;
}

bool DrawStyle::operator==(const DrawStyle& ds) const
{
    return (strokeColor_==ds.strokeColor_)
        && (fillColor_ == ds.fillColor_)
        && (strokeWidth_==ds.strokeWidth_);
}