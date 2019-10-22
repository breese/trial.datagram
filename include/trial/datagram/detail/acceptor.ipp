#ifndef TRIAL_DATAGRAM_ACCEPTOR_IPP
#define TRIAL_DATAGRAM_ACCEPTOR_IPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <utility>
#include <functional>
#include <iostream>

namespace trial
{
namespace datagram
{

inline acceptor::acceptor(const net::executor& executor,
                          endpoint_type local_endpoint)
    : boost::asio::basic_io_object<detail::service<protocol>>(static_cast<net::io_context&>(executor.context())),
      multiplexer(get_service().add(local_endpoint))
{
}

template <typename AcceptHandler>
void acceptor::async_accept(socket_type& socket,
                            AcceptHandler&& handler)
{
    assert(multiplexer);

    multiplexer->async_accept
        (socket,
         [this, &socket, handler]
         (const boost::system::error_code& error)
         {
             if (!error)
             {
                 socket.set_multiplexer(multiplexer);
                 multiplexer->add(&socket);
             }
             handler(error);
         });
}

template <typename MoveAcceptHandler>
void acceptor::async_accept(MoveAcceptHandler&& handler) {

    auto socket = std::make_shared<socket_type>(get_executor());

    this->async_accept
        (*socket,
         [socket, handler]
         (const boost::system::error_code &error) {
            std::cout << "Accepted" << std::endl;
        handler(error, std::move(*socket));
    });
}

inline acceptor::endpoint_type acceptor::local_endpoint() const
{
    assert(multiplexer);

    return multiplexer->next_layer().local_endpoint();
}

} // namespace datagram
} // namespace trial

#endif // TRIAL_DATAGRAM_ACCEPTOR_IPP
