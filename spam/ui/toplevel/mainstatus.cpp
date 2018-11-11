#include "mainstatus.h"
#include <pixmaps/green.xpm>
#include <pixmaps/red.xpm>

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

    m_statbmp = new wxStaticBitmap(this, wxID_ANY, wxIcon(green_xpm));

    m_timer.Start(1000);
    SetMinHeight(wxMax(m_statbmp->GetBestSize().GetHeight(), m_checkbox->GetBestSize().GetHeight()));

    UpdateClock();

    Bind(wxEVT_SIZE,  &MainStatus::OnSize,  this);
    Bind(wxEVT_TIMER, &MainStatus::OnTimer, this);
    Bind(wxEVT_IDLE,  &MainStatus::OnIdle,  this);
}

MainStatus::~MainStatus()
{
    if ( m_timer.IsRunning() )
    {
        m_timer.Stop();
    }
}

void MainStatus::OnSize(wxSizeEvent& event)
{
    if ( !m_checkbox )
        return;

    wxRect rect;
    if (!GetFieldRect(Field_Checkbox, rect))
    {
        event.Skip();
        return;
    }

    wxRect rectCheck = rect;
    rectCheck.Deflate(2);
    m_checkbox->SetSize(rectCheck);

    GetFieldRect(Field_Bitmap, rect);
    wxSize size = m_statbmp->GetSize();

    m_statbmp->Move(rect.x + (rect.width - size.x) / 2, rect.y + (rect.height - size.y) / 2);
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
    SetStatusText(numlockIndicators[wxGetKeyState(WXK_NUMLOCK)], Field_NumLockIndicator);
    SetStatusText(capslockIndicators[wxGetKeyState(WXK_CAPITAL)], Field_CapsLockIndicator);
    event.Skip();
}

void MainStatus::DoToggle()
{
    if ( m_checkbox->GetValue() )
    {
        m_timer.Start(1000);
        m_statbmp->SetIcon(wxIcon(green_xpm));
        UpdateClock();
    }
    else // don't show clock
    {
        m_timer.Stop();
        m_statbmp->SetIcon(wxIcon(red_xpm));
        SetStatusText(wxEmptyString, Field_Clock);
    }
}

void MainStatus::UpdateClock()
{
    SetStatusText(wxDateTime::Now().FormatTime(), Field_Clock);
}