#include "scopedtimer.h"

ScopedTimer::ScopedTimer(const wxString &label)
    : label_(label)
{
    //startTime_ = tbb::tick_count::now();
}

ScopedTimer::~ScopedTimer()
{
    //tbb::tick_count endTime = tbb::tick_count::now();
    //wxLogMessage(label_+wxT(": %fms"), (endTime - startTime_).seconds() * 1000);
}