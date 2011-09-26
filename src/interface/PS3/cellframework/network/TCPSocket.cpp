/******************************************************************************* 
 *  -- Cellframework -  Open framework to abstract the common tasks related to
 *                      PS3 application development.
 *
 *  Copyright (C) 2010
 *       Hans-Kristian Arntzen
 *       Stephen A. Damm
 *       Daniel De Matteis
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ********************************************************************************/





#include "TCPSocket.hpp"
#include "Poller.hpp"
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <string.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>

using namespace std;

namespace Network {

   TCPSocket::TCPSocket(const string& host, uint16_t port, bool in_blocking, int timeout) : Socket(SOCK_STREAM)
   {
      sockaddr_in target;
      memset(&target, 0, sizeof(target));
      
      Poller poll;
      poll.add(this, Poller::OUT);
      Poller::ListType res;

      target.sin_family = AF_INET;
      target.sin_port = htons(port);

      if (!isdigit(host[0]))
      {
         struct hostent *hostent = gethostbyname(host.c_str());
         if (hostent != NULL)
            memcpy(&target.sin_addr, hostent->h_addr, sizeof(target.sin_addr));
         else
            goto error;
      }
      else
         inet_pton(AF_INET, host.c_str(), &target.sin_addr);

      blocking(false);

      connect(fd(), (struct sockaddr*)&target, sizeof(target));
      res = poll(timeout);
      if (!(res[0].revents & Poller::OUT))
         goto error;

      blocking(in_blocking);
      return;
error:
      alive(false);
   }

   TCPSocket::TCPSocket(int fd) : Socket(SOCK_STREAM, fd) 
   {}

   ssize_t TCPSocket::recv(void *buf, size_t bytes)
   {
      if (!alive())
         return -1;

      ssize_t ret = ::recv(fd(), buf, bytes, 0);
      //std::cout << "ret: " << ret << std::endl;
      //std::cout << "blocking? " << is_blocking() << std::endl;
      if (ret <= 0 && is_blocking())
         alive(false);
      return ret;
   }

   void TCPSocket::nodelay(bool ndly)
   {
      int flag = ndly ? 1 : 0;
      setsockopt(fd(), IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
   }


   ssize_t TCPSocket::send(const void *buf, size_t bytes)
   {
      if (!alive())
         return -1;

      ssize_t ret = ::send(fd(), buf, bytes, 0);
      if (ret <= 0 && is_blocking())
         alive(false);
      return ret;
   }

   ssize_t TCPSocket::send(const char *msg)
   {
      return Socket::send(msg);
   }

   ssize_t TCPSocket::send(const std::string& msg)
   {
      return Socket::send(msg);
   }

   TCPServerSocket::TCPServerSocket(uint16_t port, bool in_blocking) : Socket(SOCK_STREAM)
   {
      sockaddr_in target;
      memset(&target, 0, sizeof(target));

      target.sin_family = AF_INET;
      target.sin_port = htons(port);
      inet_pton(AF_INET, "0.0.0.0", &target.sin_addr);

      int yes = 1;
      setsockopt(fd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

      if (bind(fd(), (struct sockaddr*)&target, sizeof(target)) < 0)
         goto error;

      if (listen(fd(), 4) < 0) // Network backlog. Just choose something
         goto error;

      blocking(in_blocking);
      return;
error:
      alive(false);
   }

   TCPSocket TCPServerSocket::accept() const
   {
      if (!alive())
         return TCPSocket(-1);

      // TODO: Make it able to fetch connection data!
      int sock = ::accept(fd(), NULL, NULL);
      if (sock < 0)
      {
         return TCPSocket(-1);
      }
      return TCPSocket(sock);
   }
}
