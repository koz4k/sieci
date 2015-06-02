#ifndef SERVICE_DISCOVERER_HPP
#define SERVICE_DISCOVERER_HPP

#include "MeasurementManager.hpp"
#include "DnsMessage.hpp"
#include "constants.hpp"
#include <boost/asio.hpp>
#include <unordered_set>

class ServiceDiscoverer
{
  public:
    ServiceDiscoverer(std::string instance, MeasurementManager& manager);

  private:
    void receive_();
    void onReceive_(const boost::system::error_code& error, size_t len);
    void discover_();
    void update_();
    void send_(const DnsMessage& message,
            const boost::asio::ip::udp::endpoint& endpoint);

    std::string instance_;
    MeasurementManager& manager_;
    std::unordered_map<std::string, const MeasurementService*> services_;
    boost::asio::deadline_timer updateTimer_;
    boost::asio::deadline_timer discoverTimer_;
    boost::asio::ip::udp::socket socket_;
    std::vector<uint8_t> buffer_;
    boost::asio::ip::udp::endpoint multicastEndpoint_;
    boost::asio::ip::udp::endpoint senderEndpoint_;
    std::unordered_map<std::string, std::pair<std::string, int>> cache_;
};

#endif
