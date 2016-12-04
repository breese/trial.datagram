#ifndef TRIAL_DATAGRAM_DETAIL_SOCKET_BASE_HPP
#define TRIAL_DATAGRAM_DETAIL_SOCKET_BASE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <memory>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/ip/udp.hpp>
#include <trial/datagram/detail/buffer.hpp>

namespace trial
{
namespace datagram
{
namespace detail
{
class multiplexer;

class socket_base
    : public boost::asio::socket_base
{
public:
    // Must be the underlying UDP types
    using endpoint_type = boost::asio::ip::udp::endpoint;

    virtual ~socket_base() = default;

    endpoint_type remote_endpoint() const { return remote; }

protected:
    friend class multiplexer;
    void remote_endpoint(const endpoint_type& r) { remote = r; }
    virtual void enqueue(const boost::system::error_code&,
                         std::unique_ptr<detail::buffer>) = 0;

protected:
    endpoint_type remote;
};

} // namespace detail
} // namespace datagram
} // namespace trial

#endif // TRIAL_DATAGRAM_DETAIL_SOCKET_BASE_HPP
