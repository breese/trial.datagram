#include <iostream>
#include <string>
#include <functional>
#include <trial/net/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <trial/datagram/socket.hpp>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }
    std::string host = argv[1];
    std::string port = argv[2];

    trial::net::io_context io;

    boost::asio::spawn
        (io,
         [&io, host, port] (boost::asio::yield_context yield)
         {
             trial::datagram::endpoint local_endpoint(trial::datagram::protocol::v4(), 0);
             trial::datagram::socket socket(trial::net::extension::get_executor(io),
                                            local_endpoint);

             boost::system::error_code error;
             socket.async_connect(host, port, yield[error]);
             if (!error)
             {
                 for (int i = 0; i < 5; ++i)
                 {
                     std::string input{"alpha"};
                     socket.async_send(boost::asio::buffer(input), yield);

                     unsigned char output[64];
                     auto length = socket.async_receive(boost::asio::buffer(output), yield);
                     for (decltype(length) i = 0; i < length; ++i)
                     {
                         std::cout << output[i];
                     }
                     std::cout << std::endl;
                 }
             }
         });

    io.run();
    return 0;
}
