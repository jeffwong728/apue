#include "mainstatus.h"
#include <pixmaps/green.xpm>
#include <pixmaps/red.xpm>
#include <wx/dcgraph.h>
#include <wx/artprov.h>

MainStatus::MainStatus(wxWindow *parent, long style)
    : wxStatusBar(parent, wxID_ANY, style, "MainStatus")
    , m_timer(this)
    , m_checkbox(NULL)
{
    const char *numlockIndicators[]  = { "OFF", "NUM" };
    const char *capslockIndicators[] = { "", "CAPS" };
    wxClientDC dc(this);
    wxSize sizeNumLock = dc.GetTextExtent(numlockIndicators[0]);
    sizeNumLock.IncTo(dc.GetTextExtent(numlockIndicators[1]));

    int widths[Field_Max];
    widths[Field_Text] = -1; // growable
    widths[Field_Checkbox] = 150;
    widths[Field_Bitmap] = 32;
    widths[Field_NumLockIndicator] = sizeNumLock.x;
    widths[Field_Clock] = 100;
    widths[Field_CapsLockIndicator] = dc.GetTextExtent(capslockIndicators[1]).x;

    SetFieldsCount(Field_Max);
    SetStatusWidths(Field_Max, widths);

    m_checkbox = new wxCheckBox(this, kSpamID_STATUS_CHECKBOX, wxT("&Toggle clock"));
    m_checkbox->SetValue(true);

    m_statbmp = new wxStaticBitmap(this, kSpamID_STATUS_CHECKBOX, wxBitmap());

    m_timer.Start(1000);
    SetMinHeight(wxMax(wxArtProvider::GetBitmap(wxART_ERROR, wxART_TOOLBAR).GetHeight(), m_checkbox->GetBestSize().GetHeight()));

    UpdateClock();

    Bind(wxEVT_SIZE,  &MainStatus::OnSize,  this);
    Bind(wxEVT_TIMER, &MainStatus::OnTimer, this);
    Bind(wxEVT_IDLE,  &MainStatus::OnIdle,  this);
    m_checkbox->Bind(wxEVT_CHECKBOX, &MainStatus::OnToggleClock, this);
}

MainStatus::~MainStatus()
{
    if ( m_timer.IsRunning() )
    {
        m_timer.Stop();
    }
}

void MainStatus::SetTextStatus(const wxString &text)
{
    SetBitmapStatus(StatusIconType::kSIT_NONE, text);
}

void MainStatus::SetBitmapStatus(const StatusIconType iconType, const wxString &text)
{
    wxRect statusRect;
    GetFieldRect(Field_Text, statusRect);

    if (statusRect.GetWidth() <= 0 || statusRect.GetHeight() <= 0)
    {
        return;
    }

    wxBitmap statusBitmap = m_statbmp->GetBitmap();
    if (!statusBitmap.IsOk() || (statusRect.GetSize() != statusBitmap.GetSize()))
    {
        statusBitmap.Create(statusRect.GetSize());
    }

    wxMemoryDC memDC(statusBitmap);
    wxGCDC dc(memDC);
    dc.SetBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    dc.Clear();

    wxBitmap iBitmap;
    wxSize   iSize;
    switch (iconType)
    {
    case StatusIconType::kSIT_ERROR:
        iBitmap = wxArtProvider::GetBitmap(wxART_ERROR, wxART_MENU);
        iSize   = iBitmap.GetSize();
        dc.DrawBitmap(iBitmap, wxPoint(0, 0));
        break;

    default:
        break;
    }

    wxFontMetrics fm = dc.GetFontMetrics();
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    dc.DrawText(text, wxPoint(iSize.GetWidth()+3, (statusRect.GetHeight()-fm.height)/2));
    memDC.SelectObject(wxNullBitmap);
    m_statbmp->SetBitmap(statusBitmap);

    m_iconType = iconType;
    m_text = text;
}

void MainStatus::OnSize(wxSizeEvent& event)
{
    wxRect statusRect;
    if (m_statbmp && GetFieldRect(Field_Text, statusRect))
    {
        m_statbmp->SetSize(statusRect);
        SetBitmapStatus(m_iconType, m_text);
    }

    wxRect rectCheck;
    if (m_checkbox && GetFieldRect(Field_Checkbox, rectCheck))
    {
        rectCheck.Deflate(2);
        m_checkbox->SetSize(rectCheck);
    }

    event.Skip();
}

void MainStatus::OnToggleClock(wxCommandEvent& WXUNUSED(event))
{
    DoToggle();
}

void MainStatus::OnIdle(wxIdleEvent& event)
{
    const char *numlockIndicators[]  = { "OFF", "NUM" };
    const char *capslockIndicators[] = { "", "CAPS" };
#ifdef __WXMSW__
    SetStatusText(numlockIndicators[wxGetKeyState(WXK_NUMLOCK)], Field_NumLockIndicator);
    SetStatusText(capslockIndicators[wxGetKeyState(WXK_CAPITAL)], Field_CapsLockIndicator);
#endif
    event.Skip();
}

void MainStatus::DoToggle()
{
    if ( m_checkbox->GetValue() )
    {
        m_timer.Start(1000);
        UpdateClock();
    }
    else // don't show clock
    {
        m_timer.Stop();
        SetStatusText(wxEmptyString, Field_Clock);
    }
}

void MainStatus::UpdateClock()
{
    SetStatusText(wxDateTime::Now().FormatTime(), Field_Clock);
}