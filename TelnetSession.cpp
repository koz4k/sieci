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
    // wylacz linemode
    buffer_ = {'\xff', '\xfd', '\x22',
               '\xff', '\xfa', '\x22', '\x01', '\x00', '\xff', '\xf0',
               '\xff', '\xfb', '\x01',
               '\r', '\n'};

    async_write(socket_, buffer(buffer_, 15),
                [this](const error_code& error, size_t len)
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

    screen_ = server_.getScreen(line_);
    async_write(socket_, buffer(screen_),
                std::bind(&TelnetSession::onWrite_, this,
                          std::placeholders::_1));
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

    bool update = false, beep = false;
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
    else if(isprint(c))
        beep = true;

    if(update)
        updateScreen();
    else if(beep)
        beep_();

    read_();
}

void TelnetSession::onWrite_(const error_code& error)
{
    if(error)
        error_("write", error);
}

void TelnetSession::beep_()
{
    buffer_ = {'\a'};
    async_write(socket_, buffer(buffer_, 1),
                std::bind(&TelnetSession::onWrite_, this,
                          std::placeholders::_1));
}

void TelnetSession::error_(const char* action, const error_code& error)
{
    std::cerr << "TelnetSession with "
              << socket_.remote_endpoint().address().to_string() << ", "
              << action << " error: " << error << std::endl;
    server_.endSession(*this);
}
