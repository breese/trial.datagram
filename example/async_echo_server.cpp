#include <iostream>
#include <memory>
#include <functional>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <trial/net/io_context.hpp>
#include <trial/datagram/acceptor.hpp>
#include <trial/datagram/socket.hpp>

class session : public std::enable_shared_from_this<session>
{
    using message_type = std::vector<char>;

public:
    session(std::shared_ptr<trial::datagram::socket> socket)
        : socket(socket)
    {
    }

    void start()
    {
        do_receive();
    }

private:
    void do_receive()
    {
        assert(socket);
        auto self(shared_from_this());
        std::shared_ptr<message_type> message = std::make_shared<message_type>(1400);
        socket->async_receive(boost::asio::buffer(*message),
                              [self, message] (boost::system::error_code error,
                                               std::size_t length)
                              {
                                  if (!error)
                                  {
                                      // Echo the message
                                      self->do_send(message, length);
                                  }
                              });
    }

    void do_send(std::shared_ptr<message_type> message,
                 std::size_t length)
    {
        assert(socket);
        auto self(shared_from_this());
        socket->async_send(boost::asio::buffer(*message, length),
                           [self, message] (boost::system::error_code error,
                                            std::size_t)
                           {
                               if (!error)
                               {
                                   self->do_receive();
                               }
                           });
    }

private:
    std::shared_ptr<trial::datagram::socket> socket;
};

class server
{
public:
    server(trial::net::io_context& io,
           const trial::datagram::endpoint& local_endpoint)
        : acceptor(trial::net::extension::get_executor(io), local_endpoint)
    {
        do_accept();
    }

    ~server()
    {
    }

private:
    void do_accept()
    {
        auto socket = std::make_shared<trial::datagram::socket>(trial::net::extension::get_executor(acceptor));
        acceptor.async_accept(*socket,
                              [this, socket] (boost::system::error_code error)
                              {
                                  if (!error)
                                  {
                                      // Session will keep itself alive as long as required
                                      std::make_shared<session>(socket)->start();
                                      do_accept();
                                  }
                              });
    }

private:
    trial::datagram::acceptor acceptor;
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    trial::net::io_context io;
    trial::datagram::endpoint endpoint(trial::datagram::protocol::v4(),
                                       std::atoi(argv[1]));
    server s(io, endpoint);
    io.run();
    return 0;
}
