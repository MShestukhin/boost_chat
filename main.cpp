//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <string>
#include <set>


using boost::asio::ip::tcp;

class chat_participant
{
public:
  virtual ~chat_participant() {}
  virtual void deliver(char& msg,std::size_t length) = 0;
};
typedef std::shared_ptr<chat_participant> chat_participant_ptr;

class chat_room
{
public:
  void join(chat_participant_ptr participant)
  {
    participants_.insert(participant);
  }

  void leave(chat_participant_ptr participant)
  {
    participants_.erase(participant);
  }

  void deliver(char &msg,std::size_t length)
  {
    for (auto participant: participants_)
      participant->deliver(msg,length);
  }

private:
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

  void deliver(char& msg,std::size_t length)
  {
      int i=0;
      memset(write_data_, 0, max_length);
      memccpy(write_data_,(&msg),'\n',length);
      do_write(length);
  }

private:
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
              std::cout<<data_<<std::endl;
              room_.deliver(*data_,length);
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
    boost::asio::async_write(socket_, boost::asio::buffer(write_data_, length),
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
  enum { max_length = 1024 };
  char data_[max_length];
  char write_data_[max_length];
  chat_room& room_;
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
    if (argc != 2)
    {
      std::cerr << "Usage: server_chat <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    server s(io_service, std::atoi(argv[1]));

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
