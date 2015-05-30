#include "MeasurementService.hpp"

std::unique_ptr<Measurer> MeasurementService::createMeasurer(
        MeasurementCollector& collector) const
{
    return measurerFactory_(collector, index_);
}
