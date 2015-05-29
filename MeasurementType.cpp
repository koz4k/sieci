#include "MeasurementType.hpp"

std::unique_ptr<Measurer> MeasurementType::createMeasurer(
        MeasurementCollector& collector) const
{
    return measurerFactory_(collector, index_);
}
