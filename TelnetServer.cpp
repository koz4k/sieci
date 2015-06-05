#include "TelnetServer.hpp"
#include "TelnetSession.hpp"
#include "constants.hpp"
#include "options.hpp"
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

TelnetServer::TelnetServer(MeasurementManager& mm):
    mm_(mm), acceptor_(io, tcp::endpoint(tcp::v4(), telnetPort)), socket_(io),
    timer_(io)
{
    accept_();
    updateScreen_();
}

std::string TelnetServer::getScreen(int startLine) const
{
    std::stringstream ss;
    ss << "\u001B[2J";  // wyczysc ekran

    int limit = std::min(SCREEN_HEIGHT - 1, (int) screen_.size() - startLine);
    for(int line = 0; line < limit; ++line)
        ss << screen_[startLine + line] << "\r\n";

    for(int line = limit; line < SCREEN_HEIGHT - 1; ++line)
        ss << "\r\n";

    return ss.str();
}

void TelnetServer::endSession(TelnetSession& session)
{
    sessions_.erase(&session);
}

void TelnetServer::accept_()
{
    acceptor_.async_accept(socket_, std::bind(&TelnetServer::onAccept_,
                           this, std::placeholders::_1));
}

void TelnetServer::onAccept_(const error_code& error)
{
    if(!error)
    {
        std::unique_ptr<TelnetSession>
                session(new TelnetSession(*this, std::move(socket_)));
        sessions_[session.get()] = std::move(session);
    }
    else
        std::cerr << "TelnetServer, accept error: " << error.message()
                  << std::endl;

    accept_();
}

void TelnetServer::updateScreen_()
{
    timer_.expires_from_now(boost::posix_time::millisec(uiRefreshPeriod * 1000));
    timer_.async_wait(std::bind(&TelnetServer::updateScreen_, this));

    bool ok = !sessions_.empty();

    std::vector<MeasurementCollector::Data> data;

    if(ok)
       data = mm_.getData();

    for(int i = 0; i < data.size(); ++i)
    {
        if(isnan(data[i].mean))
        {
            data[i--] = std::move(data.back());
            data.pop_back();
        }
    }

    if(data.empty())
        ok = false;

    if(ok)
    {
        std::sort(data.begin(), data.end());

        double maxMean = data[0].mean;

        size_t maxAddressLen = 0;
        size_t maxRenderLen = 0;
        for(const MeasurementCollector::Data& d : data)
        {
            maxAddressLen = std::max(d.address.size(), maxAddressLen);
            maxRenderLen = std::max(d.render.size(), maxRenderLen);
        }

        int space = SCREEN_WIDTH - maxAddressLen - maxRenderLen - 1;

        screen_.clear();
        for(int i = 0; i < data.size(); ++i)
        {
            std::stringstream ss;
            ss << data[i].address
               << std::string(maxAddressLen + 1 - data[i].address.size(), ' ')
               << std::string((int) (data[i].mean / maxMean * space), ' ')
               << data[i].render;
            screen_.push_back(ss.str());
        }
    }

    for(auto& p : sessions_)
        p.second->updateScreen();
}
