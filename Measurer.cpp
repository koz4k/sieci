#include "Measurer.hpp"

void Measurer::measure()
{
    startMeasurement_();
}

void Measurer::endMeasurement_(uint64_t measurement)
{
    collector_.collect(typeIndex_, measurement);
}
