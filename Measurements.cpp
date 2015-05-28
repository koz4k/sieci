#include "Measurements.hpp"
#include "constants.hpp"
#include <array>
#include <sstream>
#include <cmath>

Measurements::Data Measurements::getData() const
{
    Data data;
    data.address = address_;

    std::vector<double> means;
    for(int type = 0; type < measurements_.size(); ++type)
    {
        means.push_back(measurements_[type].empty() ? NAN :
                        ((double) sums_[type]) / measurements_.size());
    }

    std::stringstream ss;
    double sum = 0;
    int c = 0;
    for(double m : means)
    {
        Measurements::outputMeasurement_(ss, m);
        ss << " ";

        if(!isnan(m))
        {
            sum += m;
            ++c;
        }
    }

    data.render = ss.str();
    data.render = data.render.substr(0, data.render.size() - 1);

    data.mean = c ? sum / c : 0;

    return data;
}

void Measurements::collect(int type, int measurement)
{
    if(measurements_[type].size() == MEASUREMENT_MEAN_OVER)
    {
        sums_[type] -= measurements_[type].front();
        measurements_[type].pop_front();
    }

    sums_[type] += measurement;
    measurements_[type].push_back(measurement);
}

void Measurements::outputMeasurement_(std::ostream& os, double measurement)
{
    if(!isnan(measurement))
        os << (int) measurement;
    else
        os << "n/a";
}
