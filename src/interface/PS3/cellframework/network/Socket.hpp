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





#ifndef __NETWORK_SOCKET_HPP
#define __NETWORK_SOCKET_HPP

#include <unistd.h>
#include <string>
#ifdef __CELLOS_LV2__
#include <netex/net.h>
#include "network.hpp"
#endif

namespace Network {

#ifdef __CELLOS_LV2__
   class Socket : public NetworkInterface
#else
   class Socket
#endif
   {
      public:
         bool alive() const;
         bool is_blocking() const;
         void blocking(bool blocking = true);
         virtual ~Socket();

         void grab(Socket& in);
         virtual ssize_t recv(void *buf, size_t size);
         virtual ssize_t send(const void *buf, size_t size);
         ssize_t send(const std::string& msg);
         ssize_t send(const char *msg);
         bool operator==(const Socket&) const;
         int fd() const;

      protected:
         Socket(int socktype);
         Socket(int socktype, int fd);
         void alive(bool is_alive);
         void fd(int in_fd);

      private:
         void operator=(const Socket&);
         int m_fd;
         int m_type;
         bool m_alive;
   };

}

#endif
