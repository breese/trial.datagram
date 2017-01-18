///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <functional>
#include <boost/asio/error.hpp>
#include <boost/asio/io_service.hpp>
#include <trial/datagram/detail/multiplexer.hpp>

namespace trial
{
namespace datagram
{

inline socket::socket(socket&& other)
    : basic_io_object<service_type>(std::forward<decltype(other)>(other)),
      multiplexer(other.multiplexer)
{
}

inline socket::socket(boost::asio::io_service& io)
    : boost::asio::basic_io_object<service_type>(io)
{
}

inline socket::socket(boost::asio::io_service& io,
                      const endpoint_type& local_endpoint)
    : boost::asio::basic_io_object<service_type>(io),
      multiplexer(get_service().add(local_endpoint))
{
}

inline socket::~socket()
{
    if (multiplexer)
    {
        multiplexer->remove(this);
        auto local = local_endpoint();
        multiplexer.reset();
        get_service().remove(local);
    }
}

template <typename CompletionToken>
typename boost::asio::async_result<
    typename boost::asio::handler_type<CompletionToken,
                                       void(boost::system::error_code)>::type
    >::type
socket::async_connect(const endpoint_type& remote_endpoint,
                      CompletionToken&& token)
{
    using handler_type = typename boost::asio::handler_type<CompletionToken,
                                                            void(boost::system::error_code)>::type;
    handler_type handler(std::forward<decltype(token)>(token));
    boost::asio::async_result<decltype(handler)> result(handler);

    if (!multiplexer)
    {
        // Socket must be bound to a local endpoint
        invoke_handler(std::forward<decltype(handler)>(handler),
                       boost::asio::error::invalid_argument);
    }
    else
    {
        get_io_service().post(
            [this, remote_endpoint, handler] () mutable
            {
                boost::system::error_code success;
                this->process_connect(success,
                                      remote_endpoint,
                                      std::forward<decltype(handler)>(handler));
            });
    }
    return result.get();
}

template <typename ConnectHandler>
void socket::process_connect(const boost::system::error_code& error,
                             const endpoint_type& remote_endpoint,
                             ConnectHandler&& handler)
{
    // FIXME: Remove from multiplexer if already connected to different remote endpoint?
    if (!error)
    {
        assert(multiplexer);
        remote = remote_endpoint;
        multiplexer->add(this);
    }
    handler(error);
}

template <typename CompletionToken>
typename boost::asio::async_result<
    typename boost::asio::handler_type<CompletionToken,
                                       void(boost::system::error_code)>::type
    >::type
socket::async_connect(const std::string& host,
                      const std::string& service,
                      CompletionToken&& token)
{
    using handler_type = typename boost::asio::handler_type<CompletionToken,
                                                            void(boost::system::error_code)>::type;
    handler_type handler(std::forward<decltype(token)>(token));
    boost::asio::async_result<decltype(handler)> result(handler);

    if (!multiplexer)
    {
        // Socket must be bound to a local endpoint
        invoke_handler(std::forward<decltype(handler)>(handler),
                       boost::asio::error::invalid_argument);
    }
    else
    {
        std::shared_ptr<resolver_type> resolver
            = std::make_shared<resolver_type>(std::ref(get_io_service()));
        resolver_type::query query(host, service);
        resolver->async_resolve(
            query,
            [this, handler, resolver]
            (const boost::system::error_code& error, resolver_type::iterator where) mutable
            {
                 // Process resolve
                 if (error)
                 {
                     handler(error);
                 }
                 else
                 {
                     this->async_next_connect(where, resolver, handler);
                 }
             });
    }
    return result.get();
}

template <typename ConnectHandler>
void socket::async_next_connect(resolver_type::iterator where,
                                std::shared_ptr<resolver_type> resolver,
                                ConnectHandler&& handler)
{
    async_connect(
        *where,
        [this, where, resolver, handler]
        (const boost::system::error_code& error) mutable
        {
            this->process_next_connect(error,
                                       where,
                                       resolver,
                                       handler);
        });
}

template <typename ConnectHandler>
void socket::process_next_connect(const boost::system::error_code& error,
                                  resolver_type::iterator where,
                                  std::shared_ptr<resolver_type> resolver,
                                  ConnectHandler&& handler)
{
    if (error)
    {
        ++where;
        if (where == resolver_type::iterator())
        {
            // No addresses left to connect to
            handler(error);
        }
        else
        {
            // Try the next address
            async_next_connect(where, resolver, handler);
        }
    }
    else
    {
        process_connect(error, *where, std::forward<decltype(handler)>(handler));
    }
}

template <typename MutableBufferSequence,
          typename CompletionToken>
typename boost::asio::async_result<
    typename boost::asio::handler_type<CompletionToken,
                                       void(boost::system::error_code, std::size_t)>::type
    >::type
socket::async_receive(const MutableBufferSequence& buffers,
                      CompletionToken&& token)
{
    using handler_type = typename boost::asio::handler_type<CompletionToken,
                                                            void(boost::system::error_code, std::size_t)>::type;
    handler_type handler(std::forward<decltype(token)>(token));
    boost::asio::async_result<decltype(handler)> result(handler);

    if (!multiplexer)
    {
        invoke_handler(std::forward<decltype(handler)>(handler),
                       boost::asio::error::not_connected,
                       0);
    }
    else
    {
        if (receive_output_queue.empty())
        {
            std::unique_ptr<receive_input_type> operation(
                new receive_input_type(buffers,
                                       std::move(handler)));
            receive_input_queue.emplace(std::move(operation));

            multiplexer->start_receive();
        }
        else
        {
            get_io_service().post(
                [this, buffers, handler] () mutable
                {
                    auto output = std::move(this->receive_output_queue.front());
                    receive_output_queue.pop();

                    process_receive(std::get<0>(*output),
                                    *std::get<1>(*output),
                                    buffers,
                                    handler);
                });
        }
    }
    return result.get();
}

template <typename MutableBufferSequence,
          typename ReadHandler>
void socket::process_receive(const boost::system::error_code& error,
                             const detail::buffer& datagram,
                             const MutableBufferSequence& buffers,
                             ReadHandler&& handler)
{
    auto length = std::min(boost::asio::buffer_size(buffers), datagram.size());
    if (!error)
    {
        boost::asio::buffer_copy(buffers,
                                 boost::asio::buffer(datagram),
                                 length);
    }
    handler(error, length);
}

template <typename ConstBufferSequence,
          typename CompletionToken>
typename boost::asio::async_result<
    typename boost::asio::handler_type<CompletionToken,
                                       void(boost::system::error_code, std::size_t)>::type
    >::type
socket::async_send(const ConstBufferSequence& buffers,
                   CompletionToken&& token)
{
    using handler_type = typename boost::asio::handler_type<CompletionToken,
                                                            void(boost::system::error_code, std::size_t)>::type;
    handler_type handler(std::forward<decltype(token)>(token));
    boost::asio::async_result<decltype(handler)> result(handler);

    if (!multiplexer)
    {
        invoke_handler(std::forward<decltype(handler)>(handler),
                       boost::asio::error::not_connected,
                       0);
    }
    else
    {
        multiplexer->async_send_to(
            buffers,
            remote,
            [handler] (const boost::system::error_code& error,
                       std::size_t length) mutable
            {
                // Process send
                handler(error, length);
            });
    }
    return result.get();
}

template <typename Handler,
          typename ErrorCode>
void socket::invoke_handler(Handler&& handler,
                            ErrorCode error)
{
    assert(error);

    get_io_service().post(
        [handler, error]() mutable
        {
            handler(boost::asio::error::make_error_code(error));
        });
}

template <typename Handler,
          typename ErrorCode>
void socket::invoke_handler(Handler&& handler,
                            ErrorCode error,
                            std::size_t size)
{
    assert(error);

    get_io_service().post(
        [handler, error, size]() mutable
        {
            handler(boost::asio::error::make_error_code(error), size);
        });
}

inline void socket::set_multiplexer(std::shared_ptr<detail::multiplexer> value)
{
    multiplexer = value;
}

inline void socket::enqueue(const boost::system::error_code& error,
                            std::unique_ptr<detail::buffer> datagram)
{
    if (receive_input_queue.empty())
    {
        std::unique_ptr<receive_output_type> operation(
            new receive_output_type(error,
                                    std::move(datagram)));
        receive_output_queue.emplace(std::move(operation));
    }
    else
    {
        auto input = std::move(receive_input_queue.front());
        receive_input_queue.pop();

        process_receive(error,
                        *datagram,
                        std::get<0>(*input),
                        std::get<1>(*input));
    }
}

inline socket::endpoint_type socket::local_endpoint() const
{
    assert(multiplexer);

    return multiplexer->next_layer().local_endpoint();
}

inline void socket::cancel()
{
    while (!receive_input_queue.empty())
    {
        auto input = std::move(receive_input_queue.front());
        receive_input_queue.pop();

        invoke_handler(std::forward<decltype(std::get<1>(*input))>(std::get<1>(*input)),
                       boost::asio::error::operation_aborted,
                       0);
    }
}

template <typename SettableSocketOption>
void socket::set_option(const SettableSocketOption& option)
{
    boost::system::error_code error;
    set_option(option, error);
    if (error)
        throw boost::system::system_error(error);
}

template <typename SettableSocketOption>
void socket::set_option(const SettableSocketOption& option,
                        boost::system::error_code& error)
{
    assert(multiplexer);

    multiplexer->next_layer().set_option(option, error);
}

} // namespace datagram
} // namespace trial
