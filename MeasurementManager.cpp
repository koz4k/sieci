#include "MeasurementManager.hpp"

MeasurementCollector&
        MeasurementManager::startCollecting(const std::string& address)
{
    auto it = collectors_.find(address);
    if(it == collectors_.end())
    {
        it = collectors_.insert(std::make_pair(address,
                std::unique_ptr<MeasurementCollector>(
                        new MeasurementCollector(address, typeCount_)))).first;
    }
    MeasurementCollector* c = it->second.get();

    return *c;
}

std::vector<MeasurementCollector::Data> MeasurementManager::getData() const
{
    std::vector<MeasurementCollector::Data> data;
    for(const auto& p : collectors_)
        data.push_back(p.second->getData());

    return data;
}
