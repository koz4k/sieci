#include "ServiceDiscoverer.hpp"
#include "DnsMessage.hpp"
#include "io.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <functional>
#include <algorithm>
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

std::vector<std::string> splitName(const std::string& str)
{
    std::vector<std::string> name;
    int dot = 0;
    int i = str.size() - 1, j = str.size() - 1;
    while(i >= 0 && dot < 4)
    {
        if(str[i] == '.')
        {
            dot += 1;
            if(i < j)
                name.push_back(str.substr(i + 1, j - i - 1));
            j = i;
        }
        i -= 1;
    }
    name.push_back(str.substr(0, i));
    std::reverse(name.begin(), name.end());
    return name;
}

std::vector<std::string> splitService(const std::string& str)
{
    int dot = str.find('.');
    return {str.substr(0, dot), str.substr(dot + 1)};
}

std::string ipToString(const std::vector<uint8_t>& bytes)
{
    std::stringstream ss;
    for(int i = 0; i < bytes.size() - 1; ++i)
        ss << (uint32_t) bytes[i] << '.';
    ss << (uint32_t) bytes.back();
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
            services_[service.getName() + ".local."] = &service;
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
                    if(services_.find(joinName(question.name)) !=
                            services_.end())
                    {
                        std::vector<std::string> dname{instance_};
                        dname.insert(dname.end(), question.name.begin(),
                                question.name.end());
                        toSend.answers.push_back(DnsMessage::Resource(
                                std::move(question.name), question.type, 11,
                                std::move(dname)));
                    }
                    break;

                  case DnsMessage::TYPE_A:
                    if(!question.name.empty() &&
                            question.name[0] == instance_ &&
                            services_.find(joinName(question.name, 1))
                                    != services_.end())
                    {
                        ip::address addr = socket_.local_endpoint().address();
                        std::array<uint8_t, 4> bytes = addr.to_v4().to_bytes();
                        std::vector<uint8_t> data(bytes.begin(), bytes.end());
                        toSend.answers.push_back(DnsMessage::Resource(
                                std::move(question.name), question.type, 11,
                                std::move(data)));
                    }
                    break;
                }
            }
        }
        else
        {
            for(DnsMessage::Resource& answer : received.answers)
            {
                switch(answer.type)
                {
                  case DnsMessage::TYPE_PTR:
                    if(cache_.find(joinName(answer.dname)) == cache_.end())
                        toSend.questions.push_back(DnsMessage::Question(
                                std::move(answer.dname), DnsMessage::TYPE_A));
                    break;

                  case DnsMessage::TYPE_A:
                    std::string name = joinName(answer.name);
                    std::string host = ipToString(answer.data);
                    cache_[name] = std::make_pair(host, answer.ttl);
                    std::string serviceName = joinName(answer.name, 1);
                    manager_.activateServiceForHost(host,
                            services_[serviceName]);
                    break;
                }
            }
        }

        bool send = !toSend.isEmpty();
        udp::endpoint endpoint;
        if(!received.isResponse && senderEndpoint_.port() != 5353)
        {
            send = true;
            toSend.questions = received.questions;
            endpoint = senderEndpoint_;
        }
        else
            endpoint = multicastEndpoint_;

        // TODO: unicast response, delay before sending
        if(send)
            send_(toSend, endpoint);
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

    DnsMessage message;
    for(const MeasurementService& service : manager_.getServices())
    {
        std::vector<std::string> name = splitService(service.getName());
        name.push_back("local");
        message.questions.push_back(DnsMessage::Question(std::move(name),
                DnsMessage::TYPE_PTR)); // TODO: unicast response
    }

    if(!message.isEmpty())
        send_(message, multicastEndpoint_);
}

void ServiceDiscoverer::update_()
{
    updateTimer_.expires_from_now(boost::posix_time::seconds(1));
    updateTimer_.async_wait(std::bind(&ServiceDiscoverer::update_, this));

    DnsMessage query;

    auto it = cache_.begin();
    while(it != cache_.end())
    {
        it->second.second -= 1;
        if(it->second.second <= 0)
        {
            query.questions.push_back(DnsMessage::Question(
                    splitName(it->first), DnsMessage::TYPE_A));
            it = cache_.erase(it);
        }
        else
            ++it;
    }

    if(!query.isEmpty())
        send_(query, multicastEndpoint_);
}

void ServiceDiscoverer::send_(const DnsMessage& message,
        const udp::endpoint& endpoint)
{
    auto data = std::make_shared<std::vector<uint8_t>>(message.serialize());
    socket_.async_send_to(buffer(*data), endpoint,
            [data](const error_code& error, size_t len)
            {
                if(error)
                {
                    std::cerr << "ServiceDiscoverer, send error: "
                              << error.message() << std::endl;
                }
            });
}
