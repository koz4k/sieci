#include "TelnetServer.hpp"
#include "TelnetSession.hpp"
#include "constants.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <utility>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <functional>
#include <cmath>

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::tcp;

TelnetServer::TelnetServer(MeasurementCollector& mc, io_service& io,
                           uint16_t port):
    mc_(mc), acceptor_(io, tcp::endpoint(tcp::v4(), port)), socket_(io),
    timer_(io, boost::posix_time::seconds(1))
{
    accept_();
    updateScreen_();
}

std::string TelnetServer::getScreen(size_t startLine)
{
    std::stringstream ss;
    for(int line = startLine;
            line < std::min(startLine + SCREEN_HEIGHT, screen_.size());
            ++line)
        ss << screen_[line] << "\r\n";

    return ss.str();
}

void TelnetServer::accept_()
{
    acceptor_.async_accept(socket_, std::bind(&TelnetServer::onAccept_,
                           this, std::placeholders::_1));
}

void TelnetServer::onAccept_(const error_code& error)
{
    if(!error)
        new TelnetSession(std::move(socket_));
    else
        std::cerr << "TelnetServer, accept error: " << error << std::endl;

    accept_();
}

void TelnetServer::updateScreen_()
{
    std::vector<Measurements::Data> data = mc_.getData();

    if(data.empty())
    {
        screen_.clear();
        return;
    }

    std::sort(data.begin(), data.end());

    double maxMean = data[0].mean;

    size_t maxAddressLen = 0;
    size_t maxRenderLen = 0;
    for(const Measurements::Data& d : data)
    {
        maxAddressLen = std::max(d.address.size(), maxAddressLen);
        maxRenderLen = std::max(d.render.size(), maxRenderLen);
    }

    size_t space = SCREEN_WIDTH - maxAddressLen - maxRenderLen - 1;

    screen_.clear();
    for(int i = 0; i < data.size(); ++i)
    {
        std::stringstream ss;
        ss << data[i].address
           << std::string(maxAddressLen + 1 - data[i].address.size(), ' ')
           << std::string((size_t) (data[i].mean / maxMean * space), ' ')
           << data[i].render << std::endl;
        screen_.push_back(ss.str());

std::cout << screen_.back();
    }
std::cout << std::endl;

    timer_.expires_from_now(boost::posix_time::seconds(1));
    timer_.async_wait(std::bind(&TelnetServer::updateScreen_, this));
}
