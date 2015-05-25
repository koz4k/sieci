#include "TelnetServer.hpp"
#include "TelnetSession.hpp"
#include <boost/bind.hpp>
#include <utility>
#include <iostream>

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::tcp;

TelnetServer::TelnetServer(io_service& io, uint16_t port):
    io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port)), socket_(io)
{
    accept_();
}

void TelnetServer::accept_()
{
    acceptor_.async_accept(socket_, boost::bind(&TelnetServer::onAccept_,
                           this, placeholders::error));
}

void TelnetServer::onAccept_(const error_code& error)
{
    if(!error)
        new TelnetSession(std::move(socket_));
    else
        std::cerr << "TelnetServer, accept error: " << error << std::endl;

    accept_();
}
