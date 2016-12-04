#ifndef TRIAL_DATAGRAM_ENDPOINT_HPP
#define TRIAL_DATAGRAM_ENDPOINT_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <boost/asio/ip/udp.hpp>

namespace trial
{
namespace datagram
{

using protocol = boost::asio::ip::udp;
using endpoint = protocol::endpoint;

} // namespace datagram
} // namespace trial

#endif // TRIAL_DATAGRAM_ENDPOINT_HPP
