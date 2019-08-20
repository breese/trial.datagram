#ifndef TRIAL_NET_ASYNC_RESOLVE_HPP
#define TRIAL_NET_ASYNC_RESOLVE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2019 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <boost/version.hpp>

namespace trial
{
namespace net
{

template <typename Resolver, typename String, typename ResolveHandler>
void async_resolve(Resolver resolver, String&& host, String&& service, ResolveHandler&& handler)
{
#if BOOST_VERSION >= 106600 // Networking TS
    resolver->async_resolve(std::forward<decltype(host)>(host),
                            std::forward<decltype(service)>(service),
                            std::forward<decltype(handler)>(handler));
#else
    typename decltype(resolver)::element_type::query query(host, service);
    resolver->async_resolve(query,
                            std::forward<decltype(handler)>(handler));
#endif
}

} // namespace net
} // namespace trial

#endif // TRIAL_NET_ASYNC_RESOLVE_HPP
