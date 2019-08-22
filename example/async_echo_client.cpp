#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <boost/version.hpp>
#include <trial/net/io_context.hpp>
#include <boost/asio/connect.hpp>
#include <trial/datagram/socket.hpp>

class client
{
    using message_type = std::vector<char>;

public:
    client(trial::net::io_context& io,
           const std::string& host,
           const std::string& service)
        : socket(trial::net::extension::get_executor(io),
                 trial::datagram::endpoint(boost::asio::ip::udp::v4(), 0)),
          counter(4)
    {
        socket.async_connect(host,
                             service,
                             std::bind(&client::on_connect,
                                       this,
                                       std::placeholders::_1));
    }

private:
    void on_connect(const boost::system::error_code& error)
    {
        std::cout << "Connected with " << error.message() << std::endl;

        if (!error)
        {
            do_send("alpha");
        }
    }

    void do_send(const std::string& data)
    {
        std::shared_ptr<message_type> message = std::make_shared<message_type>(data.begin(), data.end());
        socket.async_send(
            boost::asio::buffer(*message),
            [this, message] (boost::system::error_code error, std::size_t length)
            {
                message->resize(length);
                if (!error)
                {
                    do_receive();
                    if (--counter > 0)
                    {
                        do_send("bravo");
                    }
                }
            });
    }

    void do_receive()
    {
        std::shared_ptr<message_type> message = std::make_shared<message_type>(1400);
        socket.async_receive(
            boost::asio::buffer(*message),
            [this, message] (boost::system::error_code error, std::size_t length)
            {
                if (!error)
                {
                    message->resize(length);
                    for (auto ch : *message)
                    {
                        std::cout << ch;
                    }
                    std::cout << std::endl;
                }
            });
    }

private:
    trial::datagram::socket socket;
    int counter;
};

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }
    trial::net::io_context io;
    client c(io, argv[1], argv[2]);
    io.run();
    return 0;
}
