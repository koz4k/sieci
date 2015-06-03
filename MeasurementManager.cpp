#include "MeasurementManager.hpp"
#include "Measurer.hpp"
#include "io.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

MeasurementManager::MeasurementManager(
        std::vector<MeasurementService> services):
    services_(std::move(services)), timer_(io)
{
    for(int index = 0; index < services_.size(); ++index)
        services_[index].setIndex(index);

    timer_.expires_from_now(boost::posix_time::seconds(1));
    timer_.async_wait(std::bind(&MeasurementManager::measure_, this));
}

const std::vector<MeasurementService>& MeasurementManager::getServices() const
{
    return services_;
}

void MeasurementManager::activateServiceForHost(const std::string& address,
        const MeasurementService* service)
{
    getCollector_(address).activateService(service);
}

void MeasurementManager::deactivateServiceForHost(const std::string& address,
        const MeasurementService* service)
{
    MeasurementCollector& collector = getCollector_(address);
    collector.deactivateService(service);
    if(collector.getActiveServiceCount() == 0)
        collectors_.erase(address);
}

MeasurementCollector& MeasurementManager::getCollector_(
        const std::string& address)
{
    auto it = collectors_.find(address);
    if(it == collectors_.end())
    {
        it = collectors_.insert(std::make_pair(address,
                std::unique_ptr<MeasurementCollector>(
                        new MeasurementCollector(address, services_)))).first;
    }
    return *it->second;
}

void MeasurementManager::measure_()
{
    timer_.expires_from_now(boost::posix_time::seconds(1));
    timer_.async_wait(std::bind(&MeasurementManager::measure_, this));
    
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
