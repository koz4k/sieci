#include "DummyMeasurer.hpp"
#include "utils.hpp"
#include <iostream>

std::unique_ptr<Measurer> DummyMeasurer::create(MeasurementCollector& collector,
        int typeIndex)
{
    return std::unique_ptr<Measurer>(new DummyMeasurer(collector, typeIndex));
}

void DummyMeasurer::startMeasurement_()
{
    int limit = 10000;
    for(int i = 0; i < limit; ++i)
        std::cout << "";

    endMeasurement_();
}
