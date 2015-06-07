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
    struct CacheEntry_
    {
        std::string address;
        int originalTtl;
        int currentTtl;

        CacheEntry_(std::string address = "", int ttl = 0):
            address(std::move(address)), originalTtl(ttl), currentTtl(ttl) {}
    };

    void receive_();
    void onReceive_(const boost::system::error_code& error, size_t len);
    void discover_();
    void update_();
    void sendOne_(const DnsMessage& message,
            const boost::asio::ip::udp::endpoint& endpoint);
    void send_(DnsMessage& message,
            const boost::asio::ip::udp::endpoint& endpoint,
            bool split = true);
    void activateServices_(const std::string& address,
            const std::string& serviceName);
    void deactivateServices_(const std::string& address,
            const std::string& serviceName);
    void regenerateInstance_();

    std::string originalInstance_;
    std::string instance_;
    int instanceId_;
    MeasurementManager& manager_;
    std::unordered_multimap<std::string,
            const MeasurementService*> services_;
    boost::asio::deadline_timer updateTimer_;
    boost::asio::deadline_timer discoverTimer_;
    boost::asio::ip::udp::socket socket_;
    std::vector<uint8_t> buffer_;
    boost::asio::ip::address_v4 myIp_;
    boost::asio::ip::address_v4 netmask_;
    boost::asio::ip::udp::endpoint multicastEndpoint_;
    boost::asio::ip::udp::endpoint senderEndpoint_;
    std::unordered_map<std::string, CacheEntry_> cache_;
    int discoveryCount_;
};

#endif
