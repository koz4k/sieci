#include "MeasurementCollector.hpp"
#include "constants.hpp"
#include <array>
#include <sstream>
#include <cmath>

MeasurementCollector::MeasurementCollector(const std::string& address,
        const std::vector<MeasurementType>& types):
    address_(address), measurements_(types.size()), sums_(types.size())
{
    for(const MeasurementType& type : types)
        measurers_.push_back(type.createMeasurer(*this));
}

void MeasurementCollector::measure()
{
    for(auto& m : measurers_)
        m->measure();
}

void MeasurementCollector::collect(int type, int measurement)
{
    if(measurements_[type].size() == MEASUREMENT_MEAN_OVER)
    {
        sums_[type] -= measurements_[type].front();
        measurements_[type].pop_front();
    }

    sums_[type] += measurement;
    measurements_[type].push_back(measurement);
}

MeasurementCollector::Data MeasurementCollector::getData() const
{
    Data data;
    data.address = address_;

    std::vector<double> means;
    for(int type = 0; type < measurements_.size(); ++type)
    {
        means.push_back(measurements_[type].empty() ? NAN :
                        ((double) sums_[type]) / measurements_[type].size());
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
