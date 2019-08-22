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
net::io_context::id service<Protocol>::id;

template <typename Protocol>
service<Protocol>::service(net::io_context& io)
    : net::io_context::service(io),
      context(io)
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
        result = std::move(detail::multiplexer::create(net::extension::get_executor(context),
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
            result = std::move(detail::multiplexer::create(net::extension::get_executor(context),
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
