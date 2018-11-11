#include "percentvalidator.h"
#include <wx/numformatter.h>

wxString PercentValidatorBase::ToString(LongestValueType value) const
{
    return wxNumberFormatter::ToString(value,
                                       m_precision,
                                       GetFormatFlags())+wxT(" %");
}

bool
PercentValidatorBase::FromString(const wxString& s, LongestValueType *value) const
{
    wxString sCopy = s.Strip(wxString::both);
    sCopy.EndsWith(wxT("%"), &sCopy);
    sCopy = sCopy.Strip(wxString::both);

    if ( !wxNumberFormatter::FromString(sCopy, value) )
        return false;

    return true;
}

bool
PercentValidatorBase::IsCharOk(const wxString& val, int pos, wxChar ch) const
{
    // We may accept minus sign if we can represent negative numbers at all.
    if ( ch == '-' )
        return false;

    const wxChar separator = wxNumberFormatter::GetDecimalSeparator();
    if ( ch == separator )
    {
        if ( val.find(separator) != wxString::npos )
        {
            // There is already a decimal separator, can't insert another one.
            return false;
        }

        // Prepending a separator before the minus sign isn't allowed.
        if ( pos == 0 && !val.empty() && val[0] == '-' )
            return false;

        // Otherwise always accept it, adding a decimal separator doesn't
        // change the number value and, in particular, can't make it invalid.
        // OTOH the checks below might not pass because strings like "." or
        // "-." are not valid numbers so parsing them would fail, hence we need
        // to treat it specially here.
        return true;
    }

    // Must be a digit then.
    if ( ch < '0' || ch > '9' )
        return false;

    // Check whether the value we'd obtain if we accepted this key is correct.
    const wxString newval(GetValueAfterInsertingChar(val, pos, ch));

    LongestValueType value;
    if ( !FromString(newval, &value) )
        return false;

    // Also check that it doesn't have too many decimal digits.
    const size_t posSep = newval.find(separator);
    if ( posSep != wxString::npos && newval.length() - posSep - 1 > m_precision )
        return false;

    // Finally check whether it is in the range.
    return IsInRange(value);
}