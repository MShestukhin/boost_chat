#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>

using boost::asio::ip::tcp;

class tcp_client
{
private:
  std::shared_ptr<tcp::socket> socket;
  std::array<char, 1024> recv_buf;

public:
  tcp_client(boost::asio::io_service& io_service)
  {
        socket = std::make_shared<tcp::socket>(io_service);
        socket->connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 1234));
        std::cout<<"Welcome to chat ! Enter your message ..."<<std::endl;
  }

  void start_receive()
  {

    socket->async_receive(boost::asio::buffer(recv_buf),
        boost::bind(&tcp_client::handle_receive, this,
          boost::asio::placeholders::error));
  }

  void write(std::string msg){
      socket->async_write_some(boost::asio::buffer(msg),
          boost::bind(&tcp_client::handler_write, this,
            boost::asio::placeholders::error));

  }
private:

  void handler_write(const boost::system::error_code &ec)
  {
      if (!ec){};
  }

  void handle_receive(const boost::system::error_code& error)
  {
//        std::cout << "handling receive: " << error << ": " << error.message() << std::endl;
        std::cout << std::string(recv_buf.data()) << std::endl;
        start_receive();
  }

};

boost::asio::io_service io_service;
tcp_client client(io_service);
static std::string username;
void read_thread()
{
    client.start_receive();
    io_service.run();
}

void write_thread(){
    while(1){
        std::string msg;
        std::getline( std::cin, msg);
        std::string res_msg=username+" : "+msg+"\n";
        client.write(res_msg);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
      std::cerr << "Usage: client_chat <user_name>\n";
      return 1;
    }
    username=std::string(argv[1]);
    boost::thread read(&read_thread);
    boost::thread write(&write_thread);
    write.join();
    read.join();
    return 0;
}
