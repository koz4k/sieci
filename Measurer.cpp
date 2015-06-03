#include "Measurer.hpp"
#include "utils.hpp"

void Measurer::measure()
{
    active_ = false;
    startTime_ = microtime();
    startMeasurement_();
}

void Measurer::endMeasurement_(int64_t measurement)
{
    if(measurement == -1)
        measurement = microtime() - startTime_;

    collector_.collect(typeIndex_, measurement);
    active_ = true;
}
