#ifndef TRIAL_DATAGRAM_DETAIL_SERVICE_IPP
#define TRIAL_DATAGRAM_DETAIL_SERVICE_IPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <trial/datagram/detail/multiplexer.hpp>

namespace trial
{
namespace datagram
{
namespace detail
{

template <typename Protocol>
boost::asio::io_service::id service<Protocol>::id;

template <typename Protocol>
service<Protocol>::service(boost::asio::io_service& io)
    : boost::asio::io_service::service(io)
{
}

template <typename Protocol>
std::shared_ptr<detail::multiplexer> service<Protocol>::add(const endpoint_type& local_endpoint)
{
    std::lock_guard<decltype(mutex)> lock(mutex);

    std::shared_ptr<detail::multiplexer> result;
    auto where = multiplexers.lower_bound(local_endpoint);
    if ((where == multiplexers.end()) || (multiplexers.key_comp()(local_endpoint, where->first)))
    {
        // Multiplexer for local endpoint does not exists
        result = std::move(detail::multiplexer::create(std::ref(get_io_service()),
                                                       local_endpoint));
        where = multiplexers.insert(
            where,
            typename decltype(multiplexers)::value_type(local_endpoint, result));
    }
    else
    {
        result = where->second.lock();
        if (!result)
        {
            // This can happen if an acceptor has failed
            // Reassign if empty
            result = std::move(detail::multiplexer::create(std::ref(get_io_service()),
                                                           local_endpoint));
            where->second = result;
        }
    }
    return result;
}

template <typename Protocol>
void service<Protocol>::remove(const endpoint_type& local_endpoint)
{
    std::lock_guard<decltype(mutex)> lock(mutex);

    auto where = multiplexers.find(local_endpoint);
    if (where != multiplexers.end())
    {
        // Only remove if multiplexer is unused
        if (!where->second.lock())
        {
            multiplexers.erase(where);
        }
    }
}

} // namespace detail
} // namespace datagram
} // namespace trial

#endif // TRIAL_DATAGRAM_DETAIL_SERVICE_IPP
