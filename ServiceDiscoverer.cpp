#include "ServiceDiscoverer.hpp"
#include "DnsMessage.hpp"
#include "io.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <functional>
#include <iostream>

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::udp;

std::string joinName(const std::vector<std::string>& name, int start = 0)
{
    std::stringstream ss;
    for(int i = start; i < name.size(); ++i)
        ss << name[i] << ".";
    return ss.str();
}

ServiceDiscoverer::ServiceDiscoverer(std::string instance,
        MeasurementManager& manager):
    instance_(std::move(instance)), manager_(manager), updateTimer_(io),
    discoverTimer_(io), socket_(io, udp::endpoint(udp::v4(), 5353)),
    buffer_(DNS_MESSAGE_MAX_LENGTH),
    multicastEndpoint_(ip::address_v4({224, 0, 0, 251}), 5353)
{
    for(const MeasurementService& service : manager.getServices())
    {
        if(!service.isHidden())
            announcedServices_.insert(service.getName() + ".local.");
    }

    socket_.set_option(ip::multicast::join_group(multicastEndpoint_.address()));
    receive_();
    discover_();
    update_();
}

void ServiceDiscoverer::receive_()
{
    socket_.async_receive_from(buffer(buffer_), senderEndpoint_,
            std::bind(&ServiceDiscoverer::onReceive_, this,
                    std::placeholders::_1, std::placeholders::_2));
}

void ServiceDiscoverer::onReceive_(const boost::system::error_code& error,
        size_t len)
{
    if(!error)
    {
        DnsMessage received(buffer_);
        DnsMessage toSend;
        if(!received.isResponse)
        {
            toSend.isResponse = true;
            for(DnsMessage::Question& question : received.questions)
            {
                switch(question.type)
                {
                  case DnsMessage::TYPE_PTR:
                    if(announcedServices_.find(joinName(question.name)) !=
                            announcedServices_.end())
                    {
                        std::vector<std::string> dname{instance_};
                        dname.insert(dname.end(), question.name.begin(),
                                question.name.end());
                        toSend.answers.push_back(DnsMessage::Resource(
                                std::move(question.name), question.type, 10,
                                std::move(dname)));
                    }
                    break;

                  case DnsMessage::TYPE_A:
                    if(!question.name.empty() &&
                            question.name[0] == instance_ &&
                            announcedServices_.find(joinName(question.name, 1))
                                    != announcedServices_.end())
                    {
                        ip::address addr = socket_.local_endpoint().address();
                        std::array<uint8_t, 4> bytes = addr.to_v4().to_bytes();
                        std::vector<uint8_t> data(bytes.begin(), bytes.end());
                        toSend.answers.push_back(DnsMessage::Resource(
                                std::move(question.name), question.type, 10,
                                std::move(data)));
                    }
                    break;
                }
            }
        }
        else
        {
            for(const DnsMessage::Resource& answer : received.answers)
            {
                // updatujemy cache
            }
        }

        bool send = !toSend.isEmpty();
        udp::endpoint endpoint;
        if(senderEndpoint_.port() != 5353)
        {
            send = true;
            endpoint = senderEndpoint_;
        }
        else
            endpoint = multicastEndpoint_;

        // TODO: unicast response
        if(send)
        {
            auto data = std::make_shared<std::vector<uint8_t>>(
                    toSend.serialize());

            socket_.async_send_to(buffer(*data), senderEndpoint_,
                    [data](const error_code& error, size_t len)
                    {
                        if(error)
                        {
                            std::cerr << "ServiceDiscoverer, send error: "
                                      << error.message() << std::endl;
                        }
                    });
        }
    }
    else
    {
        std::cerr << "ServiceDiscoverer, receive from "
                  << senderEndpoint_.address().to_string()
                  << " error: " << error.message() << std::endl;
    }

    receive_();
}

void ServiceDiscoverer::discover_()
{
    discoverTimer_.expires_from_now(boost::posix_time::seconds(10));
    discoverTimer_.async_wait(std::bind(&ServiceDiscoverer::discover_, this));

    manager_.activateServiceForHost("127.0.0.1", &manager_.getServices()[0]);
}

void ServiceDiscoverer::update_()
{
    updateTimer_.expires_from_now(boost::posix_time::seconds(1));
    updateTimer_.async_wait(std::bind(&ServiceDiscoverer::update_, this));
}
