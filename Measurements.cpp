#include "Measurements.hpp"
#include "constants.hpp"
#include <array>
#include <sstream>
#include <cmath>

Measurements::Data Measurements::getData() const
{
    Data data;
    data.address = address_;

    std::array<double, 3> means{getMean_(udp_, sumUdp_),
            getMean_(tcp_, sumTcp_), getMean_(icmp_, sumIcmp_)};

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

void Measurements::add_(std::deque<int>& dq, int& sum, int msr)
{
    if(dq.size() == MEASUREMENT_MEAN_OVER)
    {
        sum -= dq.front();
        dq.pop_front();
    }

    dq.push_back(msr);
    sum += dq.back();
}

double Measurements::getMean_(const std::deque<int>& dq, int sum)
{
    return dq.empty() ? NAN : ((double) sum) / dq.size();
}

void Measurements::outputMeasurement_(std::ostream& os, double measurement)
{
    if(!isnan(measurement))
        os << (int) measurement;
    else
        os << "n/a";
}
