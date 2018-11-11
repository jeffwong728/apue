#ifndef SPAM_UI_MISC_PERCENT_VALIDATOR_H
#define SPAM_UI_MISC_PERCENT_VALIDATOR_H
#include <wx/wxprec.h>
#include <wx/vscroll.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/valnum.h>

class PercentValidatorBase : public wxNumValidatorBase
{
public:
    void SetPrecision(unsigned precision) { m_precision = precision; }

protected:
    typedef double LongestValueType;

    PercentValidatorBase(int style)
        : wxNumValidatorBase(style)
    {
    }

    PercentValidatorBase(const PercentValidatorBase& other)
        : wxNumValidatorBase(other)
    {
        m_precision = other.m_precision;

        m_min = other.m_min;
        m_max = other.m_max;
    }

    // Provide methods for wxNumValidator use.
    wxString ToString(LongestValueType value) const;
    bool FromString(const wxString& s, LongestValueType *value) const;

    void DoSetMin(LongestValueType min) { m_min = min; }
    void DoSetMax(LongestValueType max) { m_max = max; }

    bool IsInRange(LongestValueType value) const
    {
        return m_min <= value && value <= m_max;
    }

    // Implement wxNumValidatorBase pure virtual method.
    virtual bool IsCharOk(const wxString& val, int pos, wxChar ch) const wxOVERRIDE;

private:
    // Maximum number of decimals digits after the decimal separator.
    unsigned m_precision;

    // Minimal and maximal values accepted (inclusive).
    LongestValueType m_min, m_max;

    wxDECLARE_NO_ASSIGN_CLASS(PercentValidatorBase);
};

// Validator for floating point numbers. It can be used with float, double or
// long double values.
template <typename T>
class PercentValidator
    : public wxPrivate::wxNumValidator<PercentValidatorBase, T>
{
public:
    typedef T ValueType;
    typedef wxPrivate::wxNumValidator<PercentValidatorBase, T> Base;

    // Ctor using implicit (maximal) precision for this type.
    PercentValidator(ValueType *value = NULL,
                             int style = wxNUM_VAL_DEFAULT)
        : Base(value, style)
    {
        DoSetMinMax();

        this->SetPrecision(std::numeric_limits<ValueType>::digits10);
    }

    // Ctor specifying an explicit precision.
    PercentValidator(int precision,
                      ValueType *value = NULL,
                      int style = wxNUM_VAL_DEFAULT)
        : Base(value, style)
    {
        DoSetMinMax();

        this->SetPrecision(precision);
    }

    virtual wxObject *Clone() const wxOVERRIDE
    {
        return new PercentValidator(*this);
    }

private:
    void DoSetMinMax()
    {
        // NB: Do not use min(), it's not the smallest representable value for
        //     the floating point types but rather the smallest representable
        //     positive value.
        this->DoSetMin(-std::numeric_limits<ValueType>::max());
        this->DoSetMax( std::numeric_limits<ValueType>::max());
    }
};

#endif //SPAM_UI_MISC_PERCENT_VALIDATOR_H