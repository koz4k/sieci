#ifndef MEASUREMENTS_HPP
#define MEASUREMENTS_HPP

#include <deque>
#include <ostream>

class Measurements
{
  public:
    struct Data
    {
        std::string address;
        std::string render;
        double mean;

        Data(): mean(0) {}
        bool operator<(const Data& other) { return mean > other.mean; }
    };

    Measurements(const std::string& address):
        address_(address), sumUdp_(0), sumTcp_(0), sumIcmp_(0) {}
    void addUdp(int measurement) { add_(udp_, sumUdp_, measurement); }
    void addTcp(int measurement) { add_(tcp_, sumTcp_, measurement); }
    void addIcmp(int measurement) { add_(icmp_, sumIcmp_, measurement); }
    Data getData();

  private:
    void add_(std::deque<int>& dq, int& sum, int msr);
    static double getMean_(const std::deque<int>& dq, int sum);
    double getMean_();
    static void outputMeasurement_(std::ostream& os, double measurement);

    std::string address_;
    std::deque<int> udp_, tcp_, icmp_;
    int sumUdp_, sumTcp_, sumIcmp_;
};

#endif
