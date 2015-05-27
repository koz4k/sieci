#include "Measurements.hpp"
#include "constants.hpp"
#include <array>
#include <sstream>
#include <cmath>

Measurements::Data Measurements::getData()
{
    Data data;
    data.address = address_;

    std::stringstream ss;
    Measurements::outputMeasurement_(ss,
            Measurements::getMean_(udp_, sumUdp_));
    ss << " ";
    Measurements::outputMeasurement_(ss,
            Measurements::getMean_(tcp_, sumTcp_));
    ss << " ";
    Measurements::outputMeasurement_(ss,
            Measurements::getMean_(icmp_, sumIcmp_));
    data.render = ss.str();

    data.mean = getMean_();

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

double Measurements::getMean_()
{
    std::array<double, 3> means{getMean_(udp_, sumUdp_),
            getMean_(tcp_, sumTcp_), getMean_(icmp_, sumIcmp_)};

    double sum = 0;
    int c = 0;
    for(double m : means)
    {
        if(!isnan(m))
        {
            sum += m;
            ++c;
        }
    }
    return c ? sum / c : 0;
}

void Measurements::outputMeasurement_(std::ostream& os, double measurement)
{
    if(!isnan(measurement))
        os << (int) measurement;
    else
        os << "n/a";
}

std::ostream& operator<<(std::ostream& os, Measurements& ms)
{
}
