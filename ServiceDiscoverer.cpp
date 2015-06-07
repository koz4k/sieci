#include "ServiceDiscoverer.hpp"
#include "DnsMessage.hpp"
#include "io.hpp"
#include "options.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ifaddrs.h>
#include <sstream>
#include <functional>
#include <algorithm>
#include <iostream>
#include <system_error>

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
    name.push_back(str.substr(0, i + 1));
    std::reverse(name.begin(), name.end());
    return name;
}

std::string getServiceFromName(const std::string& str)
{
    return joinName(splitName(str), 1);
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

void getMyIp(ip::address_v4& address, ip::address_v4& netmask)
{
    struct ifaddrs* ifas;
    if(getifaddrs(&ifas))
        throw std::system_error(errno, std::system_category());

    struct ifaddrs* begin = ifas;

    while(ifas)
    {
        if((ifas->ifa_flags & IFF_UP) & (ifas->ifa_flags && IFF_RUNNING) &&
                !(ifas->ifa_flags & IFF_LOOPBACK) &&
                ifas->ifa_addr->sa_family == AF_INET)
        {
            sockaddr_in* a = (sockaddr_in*) ifas->ifa_addr;
            address = ip::address_v4(ntohl(a->sin_addr.s_addr));
            a = (sockaddr_in*) ifas->ifa_netmask;
            netmask = ip::address_v4(ntohl(a->sin_addr.s_addr));
            freeifaddrs(begin);
            return;
        }

        ifas = ifas->ifa_next;
    }

    freeifaddrs(begin);
    throw std::runtime_error("No interface is connected to network");
}

ServiceDiscoverer::ServiceDiscoverer(std::string instance,
        MeasurementManager& manager):
    originalInstance_(instance), instance_(instance),
    instanceId_(2), manager_(manager), updateTimer_(io), discoverTimer_(io),
    socket_(io, udp::endpoint(udp::v4(), 5353)),
    buffer_(DNS_MESSAGE_MAX_LENGTH),
    multicastEndpoint_(ip::address_v4({224, 0, 0, 251}), 5353),
    discoveryCount_(0)
{
    getMyIp(myIp_, netmask_);

    for(const MeasurementService& service : manager.getServices())
    {
        services_.insert(std::make_pair(service.getName() + ".local.",
                &service));
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
        try
        {
            bool multicast = senderEndpoint_.port() == 5353;

            std::array<uint8_t, 4> arr = myIp_.to_bytes();
            std::vector<uint8_t> ipData(arr.begin(), arr.end());

            DnsMessage received(buffer_);

            if(senderEndpoint_.address().is_v6())
                throw std::runtime_error("querier has IPv6");

            if((myIp_.to_ulong() & netmask_.to_ulong()) !=
                    (senderEndpoint_.address().to_v4().to_ulong()
                            & netmask_.to_ulong()))
                throw std::runtime_error("querier outside of the network");

            DnsMessage toSend, toSendUnicast;
            if(!received.isResponse)
            {
                toSend.isResponse = true;
                toSendUnicast.isResponse = true;
                for(DnsMessage::Question& question : received.questions)
                {
                    DnsMessage& response = question.unicastResponse ?
                            toSendUnicast : toSend;

                    bool announce = false;
                    auto range = services_.equal_range("");
                    switch(question.type)
                    {
                      case DnsMessage::TYPE_PTR:
                        range = services_.equal_range(joinName(question.name));
                        for(auto it = range.first; it != range.second; ++it)
                        {
                            if(!it->second->isHidden())
                            {
                                announce = true;
                                break;
                            }
                        }
                        if(announce)
                        {
                            std::vector<std::string> dname{instance_};
                            dname.insert(dname.end(), question.name.begin(),
                                    question.name.end());

                            response.answers.push_back(DnsMessage::Resource(
                                    question.name, DnsMessage::TYPE_PTR,
                                    (discoveryPeriod + 1) / CACHE_ENTRY_REISSUE_AT,
                                    dname));

                            if(multicast)
                            {
                                response.answers.push_back(DnsMessage::Resource(
                                        std::move(dname), DnsMessage::TYPE_A,
                                        (discoveryPeriod + 1) / CACHE_ENTRY_REISSUE_AT,
                                        ipData));
                            }
                        }
                        break;

                      case DnsMessage::TYPE_A:
                        if(!question.name.empty() &&
                                question.name[0] == instance_ &&
                                services_.find(joinName(question.name, 1))
                                        != services_.end())
                        {
                            response.answers.push_back(DnsMessage::Resource(
                                    std::move(question.name), DnsMessage::TYPE_A,
                                    (discoveryPeriod + 1) / CACHE_ENTRY_REISSUE_AT,
                                    ipData));
                        }
                        break;
                    }
                }
            }
            else
            {
                for(DnsMessage::Resource& answer : received.answers)
                {
                    if(answer.type == DnsMessage::TYPE_A)
                    {
                        std::string name = joinName(answer.name);
                        std::string host = ipToString(answer.data);
                        CacheEntry_ oldEntry = cache_[name];
                        if(answer.ttl > oldEntry.currentTtl)
                            cache_[name] = CacheEntry_(host, answer.ttl);
                        activateServices_(host, joinName(answer.name, 1));
                    }
                }

                for(DnsMessage::Resource& answer : received.answers)
                {
                    if(answer.type == DnsMessage::TYPE_PTR)
                    {
                        if(discoveryCount_ < 2 &&
                                senderEndpoint_.address().to_v4() != myIp_ &&
                                !answer.dname.empty() &&
                                answer.dname[0] == instance_)
                        {
                            regenerateInstance_();
                        }

                        if(services_.find(joinName(answer.dname, 1)) !=
                                    services_.end()
                                && cache_.find(joinName(answer.dname)) ==
                                        cache_.end())
                        {
                            toSend.questions.push_back(DnsMessage::Question(
                                    std::move(answer.dname), DnsMessage::TYPE_A));
                        }
                    }
                }
            }

            bool send = !toSend.isEmpty();
            bool split = true;
            udp::endpoint endpoint;
            if(!received.isResponse && !multicast)
            {
                send = true;
                toSend.questions = received.questions;
                toSend.id = received.id;
                endpoint = senderEndpoint_;
                split = false;
            }
            else
            {
                if(toSend.isResponse)
                    toSend.authoritative = true;
                endpoint = multicastEndpoint_;
            }

            if(send)
                send_(toSend, endpoint, split);

            if(toSendUnicast.isResponse)
                toSendUnicast.authoritative = true;

            if(!toSendUnicast.isEmpty())
                send_(toSendUnicast, senderEndpoint_);
        }
        catch(std::exception& e)
        {
            std::cerr << "ServiceDiscoverer, receive from "
                      << senderEndpoint_.address().to_string()
                      << " error: " << e.what() << std::endl;
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
    discoverTimer_.expires_from_now(
            boost::posix_time::millisec(discoveryPeriod * 1000));
    discoverTimer_.async_wait(std::bind(&ServiceDiscoverer::discover_, this));

    std::unordered_set<std::string> asked;

    DnsMessage message;
    for(const MeasurementService& service : manager_.getServices())
    {
        if(asked.find(service.getName()) != asked.end())
            continue;

        std::vector<std::string> name = splitService(service.getName());
        name.push_back("local");
        message.questions.push_back(DnsMessage::Question(std::move(name),
                DnsMessage::TYPE_PTR, discoveryCount_ == 0));

        asked.insert(service.getName());
    }

    if(!message.isEmpty())
        send_(message, multicastEndpoint_);

    discoveryCount_ += 1;
}

void ServiceDiscoverer::update_()
{
    updateTimer_.expires_from_now(boost::posix_time::seconds(1));
    updateTimer_.async_wait(std::bind(&ServiceDiscoverer::update_, this));

    DnsMessage query;

    auto it = cache_.begin();
    while(it != cache_.end())
    {
        it->second.currentTtl -= 1;
        if(it->second.currentTtl <= 0)
        {
            deactivateServices_(it->second.address,
                    getServiceFromName(it->first));
            it = cache_.erase(it);
        }
        else
        {
            if(it->second.currentTtl ==
                    (int) it->second.originalTtl * CACHE_ENTRY_REISSUE_AT)
            {
                query.questions.push_back(DnsMessage::Question(
                        splitName(it->first), DnsMessage::TYPE_A));
            }
            ++it;
        }
    }

    if(!query.isEmpty())
        send_(query, multicastEndpoint_);
}

int getLength(const DnsMessage& message)
{
    return message.serialize().size();
}

void split(DnsMessage& toSend, DnsMessage& rest)
{
    rest = DnsMessage(toSend.isResponse, toSend.authoritative);
    while(getLength(toSend) > 512)
    {
        if(!toSend.questions.empty())
        {
            rest.questions.push_back(std::move(toSend.questions.back()));
            toSend.questions.pop_back();
        }
        else if(!toSend.answers.empty())
        {
            rest.answers.push_back(std::move(toSend.answers.back()));
            toSend.answers.pop_back();
        }
    }
}

void ServiceDiscoverer::sendOne_(const DnsMessage& message,
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

void ServiceDiscoverer::send_(DnsMessage& message,
        const udp::endpoint& endpoint, bool spl)
{
    if(!spl)
        sendOne_(message, endpoint);

    DnsMessage rest;
    do
    {
        split(message, rest);
        sendOne_(message, endpoint);
    }
    while(!rest.isEmpty());
}

void ServiceDiscoverer::activateServices_(const std::string& address,
        const std::string& serviceName)
{
    auto range = services_.equal_range(serviceName);
    for(auto it = range.first; it != range.second; ++it)
        manager_.activateServiceForHost(address, it->second);
}

void ServiceDiscoverer::deactivateServices_(const std::string& address,
        const std::string& serviceName)
{
    auto range = services_.equal_range(serviceName);
    for(auto it = range.first; it != range.second; ++it)
        manager_.deactivateServiceForHost(address, it->second);
}

void ServiceDiscoverer::regenerateInstance_()
{
    std::stringstream ss;
    ss << originalInstance_ << " (" << instanceId_++ << ")";
    instance_ = ss.str();
}
