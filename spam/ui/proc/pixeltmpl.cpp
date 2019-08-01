#include "pixeltmpl.h"
#include "basic.h"
#include <limits>
#include <stack>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267)
#include <2geom/path-sink.h>
#include <2geom/path-intersection.h>
#pragma warning( pop )
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>

PixelTemplate::PixelTemplate()
    : pyramid_level_(4)
{
}

PixelTemplate::~PixelTemplate()
{ 
}

SpamResult PixelTemplate::CreatePixelTemplate(const PixelTmplCreateData &createData)
{
    return SpamResult::kSpamResult_OK;
}

SpamResult PixelTemplate::calcCentreOfGravity(const PixelTmplCreateData &createData)
{
    if (createData.tmplRgn.empty())
    {
        return SpamResult::kSpamResult_TM_EMPTY_TEMPL_REGION;
    }

    SpamRgn l0TmplRgn;
    l0TmplRgn.AddRun(createData.tmplRgn);

    const double l0Area = l0TmplRgn.Area();
}