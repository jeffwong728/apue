#ifndef SPAM_UI_PROJS_DRAW_STYLE_H
#define SPAM_UI_PROJS_DRAW_STYLE_H
#include <wx/object.h>
#include <wx/colour.h>

class DrawStyle : public wxObject
{
public:
    DrawStyle();
    DrawStyle(const DrawStyle& ds);
    ~DrawStyle();

    DrawStyle &operator=(const DrawStyle& ds);
    bool operator==(const DrawStyle& ds) const;
    bool operator!=(const DrawStyle& ds) const { return !(*this == ds); }

public:
    wxColour strokeColor_;
    wxColour fillColor_;
    long     strokeWidth_;

private:
    wxDECLARE_DYNAMIC_CLASS(DrawStyle);
};
DECLARE_VARIANT_OBJECT(DrawStyle)
#endif //SPAM_UI_PROJS_DRAW_STYLE_H