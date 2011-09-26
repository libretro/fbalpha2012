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





#ifndef __NETWORK_POLLER_HPP
#define __NETWORK_POLLER_HPP

#include <vector>
#include <stdint.h>
#include "Socket.hpp"
#include <sys/poll.h>

namespace Network {
   
   class Poller
   {
      public:
         typedef enum PollType {
            IN = POLLIN,
            OUT = POLLOUT
         } PollType;

         struct PollFD
         {
            PollFD(Socket *in, int in_events, int in_revents = 0) : sock(in), events(in_events), revents(in_revents) {}
            Socket *sock;
            int events;
            int revents;
         };

         typedef std::vector<PollFD> ListType;
         void add(Socket *sock, int events);
         void remove(const Socket& sock);
         void clear();

         ListType operator()(int64_t msec_timeout = 0);
      private:
         ListType m_list;
   };

   namespace Internal {
      struct remove_pred
      {
         remove_pred(const Socket& sock) : m_sock(sock) {}
         bool operator()(const Poller::PollFD& in) { return *(in.sock) == m_sock; }
         const Socket& m_sock;
      };
   }

}

#endif
