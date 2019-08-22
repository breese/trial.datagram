#ifndef TRIAL_DATAGRAM_DETAIL_MULTIPLEXER_IPP
#define TRIAL_DATAGRAM_DETAIL_MULTIPLEXER_IPP

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
#include <boost/asio/buffer.hpp>
#include <trial/datagram/detail/socket_base.hpp>

namespace trial
{
namespace datagram
{
namespace detail
{

template <typename... Types>
std::unique_ptr<multiplexer> multiplexer::create(Types&&... args)
{
    std::unique_ptr<multiplexer> self(new multiplexer{std::forward<Types>(args)...});
    return self;
}

inline multiplexer::multiplexer(const net::executor& executor,
                                const endpoint_type& local_endpoint)
    : executor(executor),
      real_socket(executor, local_endpoint),
      pending_receive_count(0)
{
}

inline multiplexer::~multiplexer()
{
    assert(sockets.empty());
    assert(acceptor_queue.empty());

    listen_queue.clear();
}

inline void multiplexer::add(socket_base *socket)
{
    assert(socket);

    sockets.insert(socket_map::value_type(socket->remote_endpoint(), socket));
}

inline void multiplexer::remove(socket_base *socket)
{
    assert(socket);

    sockets.erase(socket->remote_endpoint());

    // Pending requests must receive an operation_aborted
    auto it = acceptor_queue.begin();
    while (it != acceptor_queue.end())
    {
        if (std::get<0>(**it) == socket)
        {
            auto handler = std::get<1>(**it);
            net::post(
                executor,
                [handler]
                {
                    handler(boost::asio::error::make_error_code(boost::asio::error::operation_aborted));
                });
            it = acceptor_queue.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

template <typename SocketType,
          typename AcceptHandler>
void multiplexer::async_accept(SocketType& socket,
                               AcceptHandler&& handler)
{
    // Accept requests are handled on a first-come first-serve basis

    if (listen_queue.empty())
    {
        std::unique_ptr<accept_input_type> operation(
            new accept_input_type(&socket,
                                  std::move(handler)));
        acceptor_queue.emplace_back(std::move(operation));
    }
    else
    {
        net::post(
            net::extension::get_executor(next_layer()),
            [this, &socket, handler]
            {
                assert(!listen_queue.empty());

                auto output = std::move(listen_queue.front());
                listen_queue.pop_front();

                const auto& error = std::get<0>(*output);
                if (!error)
                {
                    socket.remote_endpoint(std::get<2>(*output));
                    // Queue datagram for later use
                    socket.enqueue(error, std::move(std::get<1>(*output)));
                }
                handler(error);
            });
    }

    if (pending_receive_count++ == 0)
    {
        do_start_receive();
    }
}

template <typename ConstBufferSequence,
          typename CompletionToken>
auto multiplexer::async_send_to(const ConstBufferSequence& buffers,
                                const endpoint_type& endpoint,
                                CompletionToken&& token) -> typename net::async_result_t<CompletionToken, void(boost::system::error_code, std::size_t)>
{
    net::async_completion<CompletionToken, void(boost::system::error_code, std::size_t)> async(token);
    next_layer().async_send_to(buffers,
                               endpoint,
                               std::forward<decltype(async.completion_handler)>(async.completion_handler));
    return async.result.get();
}

inline void multiplexer::start_receive()
{
    if (pending_receive_count++ == 0)
    {
        do_start_receive();
    }
}

inline void multiplexer::do_start_receive()
{
    // Read next UDP datagram.
    //
    // If we supply a buffer whose size is smaller than the datagram, then
    // the surplus bytes will be lost. So instead we first query the size
    // asynchronously and then allocate a buffer of the appropriate size
    // which the datagram is read synchronously into.

    auto remote_endpoint = std::make_shared<endpoint_type>();
    auto self = shared_from_this();
    next_layer().async_receive_from(
        boost::asio::buffer(boost::asio::mutable_buffer()),
        *remote_endpoint,
        next_layer_type::message_peek,
        [this, self, remote_endpoint] (boost::system::error_code error, std::size_t) mutable
        {
            if (!error)
            {
                // The size_t parameter is always zero so we must query the
                // datagram size.
                next_layer_type::bytes_readable readable(true);
                next_layer().io_control(readable, error);
                if (!error)
                {
                    std::size_t length = readable.get();

                    // Datagram is available so we can read it synchronously.
                    std::unique_ptr<buffer_type> datagram(new buffer_type(length));
                    next_layer().receive_from(
                        boost::asio::buffer(datagram->data(), datagram->capacity()),
                        *remote_endpoint,
                        0,
                        error);
                    process_receive(error, std::move(datagram), *remote_endpoint);
                    return;
                }
            }
            process_receive(error, {}, *remote_endpoint);
        });
}

inline
void multiplexer::process_receive(const boost::system::error_code& error,
                                  std::unique_ptr<buffer_type> datagram,
                                  const endpoint_type& remote_endpoint)
{
    --pending_receive_count;

    if (error == boost::asio::error::operation_aborted)
        return;

    if (pending_receive_count > 0)
    {
        do_start_receive();
    }

    auto recipient = sockets.find(remote_endpoint);
    if (recipient == sockets.end())
    {
        // Unknown endpoint
        if (!acceptor_queue.empty())
        {
            // Process pending async_accept request
            auto input = std::move(acceptor_queue.front());
            acceptor_queue.pop_front();

            if (!error)
            {
                auto& socket = *std::get<0>(*input);
                socket.remote_endpoint(remote_endpoint);
                // Queue datagram for later use
                socket.enqueue(error, std::move(datagram));
            }
            std::get<1>(*input)(error); // Invoke handler
        }
        else
        {
            std::unique_ptr<accept_output_type> output(
                new accept_output_type(error,
                                       std::move(datagram),
                                       remote_endpoint));
            listen_queue.emplace_back(std::move(output));
            // FIXME: start_receive ?
        }
    }
    else
    {
        // Enqueue datagram on socket
        (*recipient).second->enqueue(error, std::move(datagram));
    }
}

inline const multiplexer::next_layer_type& multiplexer::next_layer() const
{
    return real_socket;
}

inline multiplexer::next_layer_type& multiplexer::next_layer()
{
    return real_socket;
}

} // namespace detail
} // namespace datagram
} // namespace trial

#endif // TRIAL_DATAGRAM_DETAIL_MULTIPLEXER_IPP
