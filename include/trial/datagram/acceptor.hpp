#ifndef TRIAL_DATAGRAM_ACCEPTOR_HPP
#define TRIAL_DATAGRAM_ACCEPTOR_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <trial/net/io_context.hpp>
#include <trial/datagram/detail/service.hpp>
#include <trial/datagram/endpoint.hpp>
#include <trial/datagram/socket.hpp>

namespace trial
{
namespace datagram
{

class acceptor
    : public boost::asio::basic_io_object<detail::service<protocol>>
{
public:
    using endpoint_type = trial::datagram::endpoint;
    using socket_type = trial::datagram::socket;

    acceptor(const net::executor&,
             endpoint_type local_endpoint);

    template <typename AcceptHandler>
    void async_accept(socket_type& socket,
                      AcceptHandler&& handler);

    endpoint_type local_endpoint() const;

private:
    std::shared_ptr<detail::multiplexer> multiplexer;
};

} // namespace datagram
} // namespace trial

#include <trial/datagram/detail/acceptor.ipp>

#endif // TRIAL_DATAGRAM_ACCEPTOR_HPP
