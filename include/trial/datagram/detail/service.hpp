#ifndef TRIAL_DATAGRAM_DETAIL_SERVICE_HPP
#define TRIAL_DATAGRAM_DETAIL_SERVICE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <map>
#include <mutex>
#include <trial/net/io_context.hpp>
#include <trial/datagram/endpoint.hpp>

namespace trial
{
namespace datagram
{
namespace detail
{
class multiplexer;

// One service is instantiated for trial.datagram. This service can contain
// several multiplexers, one per local endpoint, and a multiplexer can contain
// several remote endpoints.

template <typename Protocol>
class service
    : public net::io_context::service
{
    using endpoint_type = trial::datagram::endpoint;

public:
    static net::io_context::id id;

    explicit service(net::io_context& io);

    // Get or create the multiplexer that owns a local endpoint
    std::shared_ptr<detail::multiplexer> add(const endpoint_type& local_endpoint);
    void remove(const endpoint_type& local_endpoint);

    // Required by boost::asio::basic_io_object
    struct implementation_type {};
    void construct(implementation_type&) {}
    void destroy(implementation_type&) {}
    // Required for move construction
    void move_construct(implementation_type&, implementation_type&) {}
    void move_assign(implementation_type&, service&, implementation_type&) {}

private:
    virtual void shutdown_service() override {};

private:
    net::io_context& context;
    std::mutex mutex;
    std::map< endpoint_type, std::weak_ptr<detail::multiplexer> > multiplexers;
};

} // namespace detail
} // namespace datagram
} // namespace trial

#include <trial/datagram/detail/service.ipp>

#endif // TRIAL_DATAGRAM_DETAIL_SERVICE_HPP
