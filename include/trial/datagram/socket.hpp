#ifndef TRIAL_DATAGRAM_SOCKET_HPP
#define TRIAL_DATAGRAM_SOCKET_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <memory>
#include <tuple>
#include <queue>
#include <boost/asio/basic_io_object.hpp>
#include <boost/asio/ip/udp.hpp> // resolver
#include <trial/net/io_context.hpp>
#include <trial/net/async_result.hpp>
#include <trial/datagram/detail/socket_base.hpp>
#include <trial/datagram/detail/service.hpp>
#include <trial/datagram/endpoint.hpp>

namespace trial
{
namespace datagram
{
namespace detail { class multiplexer; }

class acceptor;

class socket
    : public detail::socket_base,
      public boost::asio::basic_io_object<detail::service<protocol>>
{
    using service_type = detail::service<protocol>;
    using resolver_type = protocol::resolver;

public:
    socket(socket&&);

    socket(const net::executor&);

    socket(const net::executor&,
           const endpoint_type& local_endpoint);

    virtual ~socket();

    template <typename CompletionToken>
    auto async_connect(const endpoint_type& remote_endpoint,
                       CompletionToken&& token) -> net::async_result_t<CompletionToken, void(boost::system::error_code)>;

    template <typename CompletionToken>
    auto async_connect(const std::string& remote_host,
                       const std::string& remote_service,
                       CompletionToken&& token) -> net::async_result_t<CompletionToken, void(boost::system::error_code)>;

    template <typename MutableBufferSequence,
              typename CompletionToken>
    auto async_receive(const MutableBufferSequence& buffers,
                       CompletionToken&& token) -> net::async_result_t<CompletionToken, void(boost::system::error_code, std::size_t)>;

    template <typename ConstBufferSequence,
              typename CompletionToken>
    auto async_send(const ConstBufferSequence& buffers,
                    CompletionToken&& token) -> net::async_result_t<CompletionToken, void(boost::system::error_code, std::size_t)>;

    endpoint_type local_endpoint() const;
    using detail::socket_base::remote_endpoint;

    void cancel();

    template <typename SettableSocketOption>
    void set_option(const SettableSocketOption& option);

    template <typename SettableSocketOption>
    void set_option(const SettableSocketOption& option,
                    boost::system::error_code&);

private:
    friend class detail::multiplexer;
    friend class acceptor;

    void set_multiplexer(std::shared_ptr<detail::multiplexer> multiplexer);

    virtual void enqueue(const boost::system::error_code& error,
                         std::unique_ptr<detail::buffer> datagram) override;

private:
    template <typename Handler,
              typename ErrorCode>
    void invoke_handler(Handler&& handler,
                        ErrorCode error);

    template <typename Handler,
              typename ErrorCode>
    void invoke_handler(Handler&& handler,
                        ErrorCode error,
                        std::size_t size);

    template <typename ConnectHandler>
    void process_connect(const boost::system::error_code& error,
                         const endpoint_type&,
                         ConnectHandler&& handler);

    template <typename ConnectHandler>
    void async_next_connect(resolver_type::iterator where,
                            std::shared_ptr<resolver_type> resolver,
                            ConnectHandler&& handler);

    template <typename ConnectHandler>
    void process_next_connect(const boost::system::error_code& error,
                              resolver_type::iterator where,
                              std::shared_ptr<resolver_type> resolver,
                              ConnectHandler&& handler);

    template <typename MutableBufferSequence,
              typename ReadHandler>
    void process_receive(const boost::system::error_code& error,
                         const detail::buffer& datagram,
                         const MutableBufferSequence&,
                         ReadHandler&&);

private:
    std::shared_ptr<detail::multiplexer> multiplexer;

    using read_handler_type = std::function<void (const boost::system::error_code&, std::size_t)>;
    using receive_input_type = std::tuple<boost::asio::mutable_buffer, read_handler_type>;
    using receive_output_type = std::tuple<boost::system::error_code, std::unique_ptr<detail::buffer>>;
    std::queue<std::unique_ptr<receive_input_type>> receive_input_queue;
    std::queue<std::unique_ptr<receive_output_type>> receive_output_queue;
};

} // namespace datagram
} // namespace trial

#include <trial/datagram/detail/socket.ipp>

#endif // TRIAL_DATAGRAM_SOCKET_HPP
