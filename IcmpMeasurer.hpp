#ifndef ICMP_MEASURER_HPP
#define ICMP_MEASURER_HPP

#include "Measurer.hpp"
#include <boost/asio.hpp>

class IcmpMeasurer: public Measurer
{
  public:
    IcmpMeasurer(MeasurementCollector& collector, int typeIndex);
    static std::unique_ptr<Measurer> create(MeasurementCollector& collector,
            int typeIndex);

  protected:
    void startMeasurement_();

  private:
    boost::asio::ip::icmp::endpoint endpoint_;
};

#endif
