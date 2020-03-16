#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <string>
#include <set>
#include "include/TransactionData.h"
#include "include/Block.h"
#include "include/Blockchain.h"
#include <deque>

using boost::asio::ip::tcp;
std::string* str_data;
std::vector<int> ports;
using namespace std;
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

class chat_participant
{
public:
  virtual ~chat_participant() {}
  virtual void deliver(std::string msg,std::size_t length) = 0;
};
typedef std::shared_ptr<chat_participant> chat_participant_ptr;

class chat_room
{
public:
    chat_room(){
            str_data=new std::string;
    }
  void join(chat_participant_ptr participant)
  {
    participants_.insert(participant);
  }

  void leave(chat_participant_ptr participant)
  {
    participants_.erase(participant);
  }

  void deliver(std::string msg,std::size_t length)
  {
//    time_t dataTime;
//    TransactionData data(msg, time(&dataTime));
//    awesomeCoin.addBlock(data);
//    awesomeCoin.printChain();
    for (auto participant: participants_)
      participant->deliver(msg,length);
  }

private:
//  Blockchain awesomeCoin;
  std::set<chat_participant_ptr> participants_;
};

class session
        : public chat_participant,
          public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket, chat_room& room)
    : socket_(std::move(socket)),
      room_(room)
  {

  }

  void start()
  {
    room_.join(shared_from_this());
    do_read();
  }

  void deliver(std::string msg,std::size_t length)
  {
      int i=0;
//      delete str_data;
//      str_data=new std::string;
      str_data->clear();
      for(int & port : ports)
        *str_data=*str_data+std::to_string(port)+" ";
      std::cout<<*str_data<<std::endl;
      do_write(str_data->size());
  }

private:
  void do_read()
  {
    auto self(shared_from_this());
    boost::asio::async_read_until(socket_,in_pac,'\n',[this, self](boost::system::error_code ec, std::size_t length)
    {
      if (!ec)
      {
          std::istream is(&in_pac);
          std::string msg;
          std::getline(is, msg);
          std::size_t disconnect_sustr=msg.find("disconnect:");
          if(disconnect_sustr!=std::string::npos){
              int discon_port=stoi(split(msg,":").at(1));
              for(int i=0;i<ports.size(); i++){
                  if(ports.at(i)==discon_port){
                      ports.erase(ports.cbegin()+i);
                  }
              }
          }
          else{
              ports.push_back(stoi(msg));
              room_.deliver(msg,length);
          }
          do_read();
      }
      else
      {
        room_.leave(shared_from_this());
      }
    });
  }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    std::cout<<"write "<<*str_data<<std::endl;
    boost::asio::async_write(socket_, boost::asio::buffer(*str_data),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {}
          else
          {
            room_.leave(shared_from_this());
          }
        });
  }

  tcp::socket socket_;
  enum { max_length = 10 };
  char data_[max_length];
  char write_data_[max_length];
  chat_room& room_;
  boost::asio::streambuf in_pac;
};

class server
{
public:
  server(boost::asio::io_service& io_service, short port)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      socket_(io_service)
  {
    std::cout<<"Server run..."<<std::endl;
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(socket_,
        [this](boost::system::error_code ec)
        {
          if (!ec)
          {
            std::make_shared<session>(std::move(socket_), room_)->start();
          }
          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  tcp::socket socket_;
  chat_room room_;
};

int main(int argc, char* argv[])
{
  try
  {

    boost::asio::io_service io_service;

    server s(io_service, std::atoi("2222"));

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
