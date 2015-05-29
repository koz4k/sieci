#include "MeasurementManager.hpp"
#include "Measurer.hpp"

MeasurementManager::MeasurementManager(std::vector<MeasurementType> types):
    types_(std::move(types))
{
    for(int index = 0; index < types_.size(); ++index)
        types_[index].setIndex(index);
}

void MeasurementManager::startCollecting(const std::string& address)
{
    auto it = collectors_.find(address);
    if(it == collectors_.end())
    {
        it = collectors_.insert(std::make_pair(address,
                std::unique_ptr<MeasurementCollector>(
                        new MeasurementCollector(address, types_)))).first;
    }
}

void MeasurementManager::measure()
{
    for(auto& p : collectors_)
        p.second->measure();
}

std::vector<MeasurementCollector::Data> MeasurementManager::getData() const
{
    std::vector<MeasurementCollector::Data> data;
    for(const auto& p : collectors_)
        data.push_back(p.second->getData());

    return data;
}
