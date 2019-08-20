#ifndef TRIAL_NET_IO_CONTEXT_HPP
#define TRIAL_NET_IO_CONTEXT_HPP

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
#if BOOST_VERSION >= 106600
# define TRIAL_NET_HAVE_IO_CONTEXT
#endif

#if defined(TRIAL_NET_HAVE_IO_CONTEXT)
# include <boost/asio/io_context.hpp>
# include <boost/asio/post.hpp>
#else
# include <boost/asio/io_service.hpp>
#endif

namespace trial
{
namespace net
{

#if defined(TRIAL_NET_HAVE_IO_CONTEXT)

using io_context = boost::asio::io_context;
using executor = io_context::executor_type;

using boost::asio::get_associated_executor;

template <typename T>
auto get_context(T& t) -> io_context&
{
    return t.context();
}

template <typename T>
auto get_executor(T&& t) -> decltype(t.get_executor())
{
    return t.get_executor();
}

using boost::asio::post;

#else

using io_context = boost::asio::io_service;

class executor
{
public:
    executor() = default;
    executor(const executor&) = default;
    executor(executor&&) = default;
    executor& operator=(const executor&) = default;
    executor& operator=(executor&&) = default;

    executor(io_context& context)
        : context(&context)
    {
    }

    operator io_context&() const
    {
        return *context;
    }

    operator io_context&()
    {
        return *context;
    }

    template <typename... Args>
    void post(Args&&... args) const
    {
        context->post(std::forward<Args>(args)...);
    }

private:
    io_context *context = nullptr;
};

template <typename T>
auto get_context(T& t) -> io_context&
{
    return t.get_io_service();
}

template <>
auto get_context(executor& t) -> io_context&
{
    return static_cast<io_context&>(t);
}

template <>
auto get_context(const executor& t) -> io_context&
{
    return static_cast<io_context&>(t);
}

template <typename T>
auto get_executor(T& t) -> executor
{
    return executor(get_context(t));
}

template <>
auto get_executor(io_context& t) -> executor
{
    return executor(t);
}

template <typename Executor, typename CompletionToken>
auto post(Executor&& executor,
          CompletionToken&& token) -> decltype(get_context(executor).post(std::forward<decltype(token)>(token)))
{
    return get_context(executor).post(std::forward<decltype(token)>(token));
}

#endif

} // namespace net
} // namespace trial

#endif // TRIAL_NET_IO_CONTEXT_HPP
