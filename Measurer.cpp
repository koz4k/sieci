#include "Measurer.hpp"

void Measurer::measure()
{
    active_ = false;
    startMeasurement_();
}

void Measurer::endMeasurement_(uint64_t measurement)
{
    collector_.collect(typeIndex_, measurement);
    active_ = true;
}
