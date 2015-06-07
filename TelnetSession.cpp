#include "TelnetSession.hpp"
#include "constants.hpp"
#include <utility>
#include <functional>
#include <iostream>
#include <cctype>

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::tcp;

TelnetSession::TelnetSession(TelnetServer& server, tcp::socket socket):
    server_(server), socket_(std::move(socket)), ready_(false), line_(0)
{
    // wylacz linemode i go-ahead
    auto data = std::shared_ptr<std::vector<char>>(new std::vector<char>{
            '\xff', '\xfd', '\x22',                 // IAC DO LINEMODE
            '\xff', '\xfa', '\x22', '\x01', '\x00', // IAC SB LINEMODE MODE 0
            '\xff', '\xf0',                         // IAC SE
            '\xff', '\xfb', '\x01',                 // IAC WILL ECHO
            '\xff', '\xfb', '\x03',             // IAC WILL SUPPRESS-GO-AHEAD
            '\r', '\n'
    });
    async_write(socket_, buffer(*data),
            [this, data](const error_code& error, size_t len)
            {
                if(error)
                    error_("write", error);
                else
                    ready_ = true;

                read_();
            });
}

void TelnetSession::updateScreen()
{
    if(!ready_)
        return;

    auto data = std::make_shared<std::string>(server_.getScreen(line_));
    async_write(socket_, buffer(*data),
            [this, data](const error_code& error, size_t len)
            {
                if(error)
                    error_("write", error);
            });
}

void TelnetSession::read_()
{
    async_read(socket_, buffer(buffer_, 1),
            std::bind(&TelnetSession::onRead_, this,
                      std::placeholders::_1,
                      std::placeholders::_2));
}

void TelnetSession::onRead_(const error_code& error, size_t len)
{
    if(error)
    {
        error_("read", error);
        return;
    }

    int limit = std::max(server_.getScreenHeight() - SCREEN_HEIGHT + 1, 0);

    bool update = false;
    char c = buffer_[0];
    if(c == 'q' && line_ > 0)
    {
        line_ -= 1;
        update = true;
    }
    else if(c == 'a' && line_ < limit)
    {
        line_ += 1;
        update = true;
    }

    if(update)
        updateScreen();

    read_();
}

void TelnetSession::error_(const char* action, const error_code& error)
{
    try
    {
        std::cerr << "TelnetSession assigned to "
                  << socket_.remote_endpoint().address().to_string() << ", "
                  << action << " error: " << error.message() << std::endl;
    }
    catch(system_error& e)
    {
        std::cerr << "TelnetSession, " << action << " error: "
                  << error.message() << std::endl;
    }

    server_.endSession(*this);
}
