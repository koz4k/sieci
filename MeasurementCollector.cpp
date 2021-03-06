#include "MeasurementCollector.hpp"
#include "constants.hpp"
#include <array>
#include <sstream>
#include <cmath>

MeasurementCollector::MeasurementCollector(const std::string& address,
        const std::vector<MeasurementService>& services):
    address_(address), measurements_(services.size()), sums_(services.size()),
    activeServiceCount_(0)
{
    for(const MeasurementService& service : services)
        measurers_.push_back(service.createMeasurer(*this));
}

void MeasurementCollector::activateService(const MeasurementService* service)
{
    int index = service->getIndex();
    if(!measurers_[index]->isActive())
    {
        measurers_[index]->setActive(true);
        activeServiceCount_ += 1;
    }
}

void MeasurementCollector::deactivateService(const MeasurementService* service)
{
    int index = service->getIndex();
    if(measurers_[index]->isActive())
    {
        measurers_[index]->setActive(false);
        measurements_[index].clear();
        sums_[index] = 0;
        activeServiceCount_ -= 1;
    }
}

void MeasurementCollector::measure()
{
    for(auto& m : measurers_)
        if(m->isReady())
            m->measure();
}

void MeasurementCollector::collect(int index, int measurement)
{
    if(index < 0 || index >= measurements_.size())
        return;

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
    {
        std::vector<std::string> suffixes{"us", "ms", "s"};
        int i = 0;
        for(; measurement >= 1000 && i < suffixes.size(); ++i)
            measurement /= 1000;
        
        os.precision(3);
        os << measurement << suffixes[i];
    }
    else
        os << "n/a";
}
