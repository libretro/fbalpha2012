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





#include "Poller.hpp"
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <algorithm>

namespace Network {

   void Poller::add(Socket *sock, int events)
   {
      m_list.push_back(PollFD(sock, events));
   }

   void Poller::remove(const Socket &sock)
   {
      m_list.erase(std::remove_if(m_list.begin(), m_list.end(), Internal::remove_pred(sock)), m_list.end());
   }

   void Poller::clear()
   {
      m_list.clear();
   }

   Poller::ListType Poller::operator()(int64_t msec_timeout)
   {
      struct pollfd *poll_fd = new struct pollfd[m_list.size()];
      for (size_t i = 0; i < m_list.size(); i++)
      {
         poll_fd[i].fd = m_list[i].sock->fd();
         poll_fd[i].events = m_list[i].events;
      }

#ifdef __CELLOS_LV2__
      socketpoll(poll_fd, m_list.size(), msec_timeout);
#else
      poll(poll_fd, m_list.size(), msec_timeout);
#endif

      for (size_t i = 0; i < m_list.size(); i++)
         m_list[i].revents = poll_fd[i].revents;

      delete[] poll_fd;
      return m_list;
   }
}
