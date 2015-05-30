#include "MeasurementManager.hpp"
#include "Measurer.hpp"
#include "io.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

MeasurementManager::MeasurementManager(std::vector<MeasurementType> types):
    types_(std::move(types)), timer_(io)
{
    for(int index = 0; index < types_.size(); ++index)
        types_[index].setIndex(index);

    timer_.expires_from_now(boost::posix_time::seconds(1));
    timer_.async_wait(std::bind(&MeasurementManager::measure_, this));
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

void MeasurementManager::measure_()
{
    timer_.expires_from_now(boost::posix_time::seconds(1));
    timer_.async_wait(std::bind(&MeasurementManager::measure_, this));
    
    for(int i = 0; i < 10; ++i)
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
