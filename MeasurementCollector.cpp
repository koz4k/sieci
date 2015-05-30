#include "MeasurementCollector.hpp"
#include "constants.hpp"
#include <array>
#include <sstream>
#include <cmath>

MeasurementCollector::MeasurementCollector(const std::string& address,
        const std::vector<MeasurementService>& services):
    address_(address), measurements_(services.size()), sums_(services.size())
{
    for(const MeasurementService& service : services)
    {
        measurers_.push_back(service.createMeasurer(*this));
        measurers_.back()->setActive(true);
    }
}

void MeasurementCollector::activateService(const MeasurementService* service)
{
    measurers_[service->getIndex()]->setActive(true);
}

void MeasurementCollector::setActiveServices(
        const std::vector<const MeasurementService*>& services)
{
    int j = 0;
    for(int i = 0; i < services.size(); ++i)
    {
        while(j < services[i]->getIndex())
            measurers_[j++]->setActive(false);
        measurers_[j++]->setActive(true);
    }

    while(j < measurers_.size())
        measurers_[j++]->setActive(false);
}

void MeasurementCollector::measure()
{
    for(auto& m : measurers_)
        if(m->isActive())
            m->measure();
}

void MeasurementCollector::collect(int index, int measurement)
{
    if(measurements_[index].size() == MEASUREMENT_MEAN_OVER)
    {
        sums_[index] -= measurements_[index].front();
        measurements_[index].pop_front();
    }

    sums_[index] += measurement;
    measurements_[index].push_back(measurement);
}

MeasurementCollector::Data MeasurementCollector::getData() const
{
    Data data;
    data.address = address_;

    std::vector<double> means;
    for(int index = 0; index < measurements_.size(); ++index)
    {
        means.push_back(measurements_[index].empty() ? NAN :
                        ((double) sums_[index]) / measurements_[index].size());
    }

    std::stringstream ss;
    double sum = 0;
    int c = 0;
    for(double m : means)
    {
        outputMeasurement_(ss, m);
        ss << " ";

        if(!isnan(m))
        {
            sum += m;
            ++c;
        }
    }

    data.render = ss.str();
    data.render = data.render.substr(0, data.render.size() - 1);

    data.mean = c ? sum / c : NAN;

    return data;
}

void MeasurementCollector::outputMeasurement_(std::ostream& os,
        double measurement)
{
    if(!isnan(measurement))
        os << (int) measurement;
    else
        os << "n/a";
}
