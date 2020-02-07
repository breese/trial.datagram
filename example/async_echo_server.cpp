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
    session(trial::datagram::socket&& socket)
        : socket(std::move(socket))
    {
    }

    void start()
    {
        do_receive();
    }

private:
    void do_receive()
    {
        auto self(shared_from_this());
        std::shared_ptr<message_type> message = std::make_shared<message_type>(1400);
        socket.async_receive(boost::asio::buffer(*message),
                              [self, message] (boost::system::error_code error,
                                               std::size_t length)
                              {
                                  std::cout << "do_receive: received=" << message << std::endl;
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
        auto self(shared_from_this());
        socket.async_send(boost::asio::buffer(*message, length),
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
    trial::datagram::socket socket;
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
        std::cout << "Accepting..." << std::endl;
        acceptor.async_accept([this] (boost::system::error_code error, trial::datagram::socket socket)
                              {
                                  if (!error)
                                  {
                                      std::cout << "async_echo_server: Accepted" << std::endl;
                                      // Session will keep itself alive as long as required
                                      std::make_shared<session>(std::move(socket))->start();
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
