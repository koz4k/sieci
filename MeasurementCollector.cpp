#include "MeasurementCollector.hpp"

Measurements& MeasurementCollector::collect(const std::string& address)
{
    auto it = measurements_.find(address);
    if(it == measurements_.end())
    {
        it = measurements_.insert(std::make_pair(address,
                std::unique_ptr<Measurements>(
                        new Measurements(address)))).first;
    }
    Measurements* m = it->second.get();

    return *m;
}

std::vector<Measurements::Data> MeasurementCollector::getData() const
{
    std::vector<Measurements::Data> ms;
    for(const auto& p : measurements_)
        ms.push_back(p.second->getData());

    return ms;
}
