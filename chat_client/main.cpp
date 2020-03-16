#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using namespace std;
std::vector<std::string> ports;
int port;

vector<string> split(const string& str, const string& delim)
{
    vector<string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

class tcp_client
{
private:
  std::shared_ptr<tcp::socket> socket;
  std::array<char, 1024> recv_buf;

public:
  tcp_client(boost::asio::io_service& io_service)
  {
        socket = std::make_shared<tcp::socket>(io_service);
        socket->connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 2222));
        std::cout<<"Welcome to chat ! Enter your message ..."<<std::endl;
  }
  ~tcp_client(){
      std::string disconect_str="disconnect:"+std::to_string(port)+"\n";
      write(disconect_str);
      socket->close();
  }

  void start_receive()
  {
    memset(recv_buf.data(),0,1024);
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
        std::cout << std::string(recv_buf.data()) << std::endl;
        ports.clear();
        std::string ports_str=std::string(recv_buf.data());
        ports=split(ports_str, " ");
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

char buff[1024];
boost::asio::ip::udp::socket *sock;
void write_thread(){
        std::string res_msg=std::to_string(port)+"\n";
        client.write(res_msg);
        while ( true)
        {
            boost::asio::ip::udp::endpoint sender_ep;
            int bytes = sock->receive_from(boost::asio::buffer(buff), sender_ep);
            std::string msg(buff, bytes);
            std::cout<<msg<<std::endl;
            memset(buff,0,1024);
        }

}

void peer_write_thread(){
    while(1){
        std::string msg;
        std::getline(std::cin, msg);
        if(msg=="exit"){
            std::exit(EXIT_SUCCESS);
        }
        std::string res_msg=msg+"\n";
        for(string & peer_port : ports){
            boost::asio::ip::udp::endpoint sender_ep(boost::asio::ip::udp::v4(), stoi(peer_port));
            sock->send_to(boost::asio::buffer(msg), sender_ep);
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
      std::cerr << "Usage: client_chat <user_name>\n";
      return 1;
    }
    port=atoi(argv[1]);
    sock = new boost::asio::ip::udp::socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));
    boost::thread read(&read_thread);
    boost::thread write(&write_thread);
    boost::thread peer_write(&peer_write_thread);
    write.join();
    read.join();
    peer_write.join();
    delete sock;
    return 0;
}
