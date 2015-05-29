#include "Measurer.hpp"
#include "utils.hpp"

void Measurer::measure()
{
    startTime_ = microtime();
    startMeasurement_();
}

void Measurer::endMeasurement_()
{
    collector_.collect(typeIndex_, microtime() - startTime_);
}
