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





#ifndef __NETWORK_TCPSOCKET_HPP
#define __NETWORK_TCPSOCKET_HPP

#include <stddef.h>
#include <stdint.h>
#include <string>
#include "Socket.hpp"

namespace Network {

   class TCPSocket : public Socket
   {
      public:
         TCPSocket(const std::string& host, uint16_t port, bool blocking = true, int timeout = 3000);
         TCPSocket(int fd = -1);

         ssize_t recv(void *buf, size_t bytes);
         ssize_t send(const void *buf, size_t bytes);
         ssize_t send(const std::string& msg);
         ssize_t send(const char *msg);
         void nodelay(bool);

      private:
         void operator=(const TCPSocket&); // Can't copy a socket directly
   };

   class TCPServerSocket : public Socket
   {
      public:
         TCPServerSocket(uint16_t port, bool blocking = true);
         TCPSocket accept() const;
      private:
         TCPServerSocket(const TCPServerSocket&); // Can't copy a socket
         void operator=(const TCPServerSocket&);
   };
}

#endif
