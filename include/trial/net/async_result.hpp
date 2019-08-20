#ifndef TRIAL_NET_ASYNC_RESULT_HPP
#define TRIAL_NET_ASYNC_RESULT_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2019 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <boost/asio/async_result.hpp>

namespace trial
{
namespace net
{

#if BOOST_VERSION >= 107000

template <typename CompletionToken, typename Signature>
using async_result = boost::asio::async_result<typename std::decay<CompletionToken>::type, Signature>;

template <typename CompletionToken, typename Signature>
using async_result_t = typename async_result<CompletionToken, Signature>::return_type;

template <typename CompletionToken, typename Signature>
using async_completion = boost::asio::async_completion<CompletionToken, Signature>;

#else

// Deprecated in Boost.Asio 1.67, removed from 1.70

template <typename CompletionToken, typename Signature>
using async_result = boost::asio::async_result<typename boost::asio::handler_type<CompletionToken, Signature>::type>;

template <typename CompletionToken, typename Signature>
using async_result_t = typename async_result<CompletionToken, Signature>::type;

template <typename CompletionToken, typename Signature>
struct async_completion
{
    async_completion(CompletionToken& token)
        : completion_handler(token),
          result(completion_handler)
    {
    }

    using handler_type = typename boost::asio::handler_type<CompletionToken, Signature>::type;
    handler_type completion_handler;
    boost::asio::async_result<handler_type> result;
};

#endif

} // namespace net
} // namespace trial

#endif // TRIAL_NET_ASYNC_RESULT_HPP
