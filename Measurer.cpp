#include "Measurer.hpp"
#include "utils.hpp"

void Measurer::measure()
{
    active_ = false;
    startTime_ = microtime();
    startMeasurement_();
}

void Measurer::endMeasurement_()
{
    collector_.collect(typeIndex_, microtime() - startTime_);
    active_ = true;
}
