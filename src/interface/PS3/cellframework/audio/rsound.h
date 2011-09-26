/*  RSound - A PCM audio client/server
 *  Copyright (C) 2010 - Hans-Kristian Arntzen
 * 
 *  RSound is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RSound is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RSound.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __RSOUND_H
#define __RSOUND_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RSD_EXPOSE_STRUCT
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stddef.h>
#else
#include <stddef.h>
#endif

#ifdef _WIN32
#define RSD_DEFAULT_HOST "127.0.0.1" // Stupid Windows.
#else
#define RSD_DEFAULT_HOST "localhost"
#endif
#define RSD_DEFAULT_PORT "12345"
#define RSD_DEFAULT_UNIX_SOCK "/tmp/rsound"
#define RSD_DEFAULT_OBJECT "rsound"

#ifndef RSD_VERSION
#define RSD_VERSION "1.0beta1"
#endif

/* Feature tests */
#define RSD_SAMPLERATE              RSD_SAMPLERATE 
#define RSD_CHANNELS                RSD_CHANNELS
#define RSD_HOST                    RSD_HOST
#define RSD_PORT                    RSD_PORT
#define RSD_BUFSIZE                 RSD_BUFSIZE
#define RSD_LATENCY                 RSD_LATENCY
#define RSD_FORMAT                  RSD_FORMAT
#define RSD_IDENTITY                RSD_IDENTITY

#define RSD_S16_LE                  RSD_S16_LE
#define RSD_S16_BE                  RSD_S16_BE
#define RSD_U16_LE                  RSD_U16_LE
#define RSD_U16_BE                  RSD_U16_BE
#define RSD_U8                      RSD_U8
#define RSD_S8                      RSD_S8
#define RSD_S16_NE                  RSD_S16_NE
#define RSD_U16_NE                  RSD_U16_NE
#define RSD_ALAW                    RSD_ALAW
#define RSD_MULAW                   RSD_MULAW

#define RSD_DELAY_MS                RSD_DELAY_MS
#define RSD_SAMPLESIZE              RSD_SAMPLESIZE
#define RSD_EXEC                    RSD_EXEC
#define RSD_SIMPLE_START            RSD_SIMPLE_START

#define RSD_NO_FMT                  RSD_NO_FMT
#define RSD_USES_OPAQUE_TYPE        RSD_USES_OPAQUE_TYPE
#define RSD_USES_SAMPLESIZE_MEMBER  RSD_USES_SAMPLESIZE_MEMBER
/* End feature tests */



   /* Defines sample formats available. Defaults to S16_LE should it never be set. */
   enum rsd_format
   {
      RSD_NO_FMT = 0x0000,
      RSD_S16_LE = 0x0001,
      RSD_S16_BE = 0x0002,
      RSD_U16_LE = 0x0004,
      RSD_U16_BE = 0x0008,
      RSD_U8     = 0x0010,
      RSD_S8     = 0x0020,
      RSD_S16_NE = 0x0040,
      RSD_U16_NE = 0x0080,
      RSD_ALAW   = 0x0100,
      RSD_MULAW  = 0x0200
   };

   /* Defines operations that can be used with rsd_set_param() */
   enum rsd_settings
   {
      RSD_SAMPLERATE = 0,
      RSD_CHANNELS,
      RSD_HOST,
      RSD_PORT,
      RSD_BUFSIZE,
      RSD_LATENCY,
      RSD_FORMAT,
      RSD_IDENTITY
   };


#ifdef RSD_EXPOSE_STRUCT

#ifndef RSD_FIFO_BUF_TYPEDEF
#define RSD_FIFO_BUF_TYPEDEF
   typedef struct rsound_fifo_buffer rsound_fifo_buffer_t;
#endif

   /* Defines the main structure for use with the API. */
   typedef struct rsound
   {
      struct {
         volatile int socket;
         volatile int ctl_socket;
      } conn;

      char *host;
      char *port;
      char *buffer; // Obsolete, but kept for backwards header compatibility.
      int conn_type;

      volatile int buffer_pointer; // Obsolete, but kept for backwards header compatibility.
      size_t buffer_size; 
      rsound_fifo_buffer_t *fifo_buffer;

      volatile int thread_active;

      int64_t total_written;
      int64_t start_time;
      volatile int has_written;
      int bytes_in_buffer;
      int delay_offset;
      int max_latency;

      struct {
         uint32_t latency;
         uint32_t chunk_size;
      } backend_info;

      volatile int ready_for_data;

      uint32_t rate;
      uint32_t channels;
      uint16_t format;
      int samplesize;

      struct {
         pthread_t threadId;
         pthread_mutex_t mutex;
         pthread_mutex_t cond_mutex;
         pthread_cond_t cond;
      } thread;

      char identity[256];
   } rsound_t;
#else
   typedef struct rsound rsound_t;
#endif

   /* -- API --
      All functions (except for rsd_write() return 0 for success, and -1 for error. errno is currently not set. */

   /* Initializes an rsound_t structure. To make sure no memory leaks occur, you need to rsd_free() it after use.
   A typical use of the API is as follows:
      rsound_t *rd;
      rsd_init(&rd);
      rsd_set_param(rd, RSD_HOST, "foohost");
      *sets more params*
      rsd_start(rd);
      rsd_write(rd, buf, size); 
      rsd_stop(rd);
      rsd_free(rd);
    */
   int rsd_init (rsound_t **rd);


   /* This is a simpler function that initializes an rsound struct, sets params as given, 
      and starts the stream. Should this function fail, the structure will stay uninitialized.
      Should NULL be passed in either host, port or ident, defaults will be used. */

   int rsd_simple_start (rsound_t **rd, const char* host, const char* port, const char* ident,
                           int rate, int channels, enum rsd_format format);


   /* Sets params associated with an rsound_t. These options (int options) include:

   RSD_HOST: Server to connect to. Expects (char *) in param. 
   If not set, will default to environmental variable RSD_SERVER or "localhost". 

   RSD_PORT: Set port. Expects (char *) in param. 
   If not set, will default to environmental variable RSD_PORT or "12345".

   RSD_CHANNELS: Set number of audio channels. Expects (int *) in param. Mandatory.

   RSD_SAMPLERATE: Set samplerate of audio stream. Expects (int *) in param. Mandatory.

   RSD_BUFSIZE: Sets internal buffersize for the stream. 
   Might be overridden if too small. 
   Expects (int *) in param. Optional.

   RSD_LATENCY: Sets maximum audio latency in milliseconds, 
   (must be used with rsd_delay_wait() or this will have no effect). 
   Most applications do not need this. 
   Might be overridden if too small. 
   Expects (int *) in param. Optional.

   RSD_FORMAT: Sets sample format. 
   It defaults to S16_LE, so you probably will not use this. 
   Expects (int *) in param, with available values found in the format enum. 
   If invalid format is given, param might be changed to reflect the sample format the library will use.

   RSD_IDENTITY: Sets an identity string associated with the client.
   Takes a (char *) parameter with the stream name.
   Will be truncated if longer than 256 bytes.

   */

   int rsd_set_param (rsound_t *rd, enum rsd_settings option, void* param);

   /* Establishes connection to server. Might fail if connection can't be established or that one of 
      the mandatory options isn't set in rsd_set_param(). This needs to be called after params have been set
      with rsd_set_param(), and before rsd_write(). */ 
   int rsd_start (rsound_t *rd);

   /* Shuts down the rsound data structures, but returns the file descriptor associated with the connection.
      The control socket will be shut down. If this function returns a negative number, the exec failed, 
      but the data structures will not be teared down. 
      Should a valid file descriptor be returned, it will always be blocking.
      This call will block until all internal buffers have been sent to the network.  */
   int rsd_exec (rsound_t *rd);

   /* Disconnects from server. All audio data still in network buffer and other buffers will be dropped. 
      To continue playing, you will need to rsd_start() again. */
   int rsd_stop (rsound_t *rd);

   /* Writes from buf to the internal buffer. Might fail if no connection is established, 
      or there was an unexpected error. This function will block until all data has
      been written to the buffer. This function will return the number of bytes written to the buffer,
      or 0 should it fail (disconnection from server). You will have to restart the stream again should this occur. */
   size_t rsd_write (rsound_t *rd, const void* buf, size_t size);

   /* Gets the position of the buffer pointer. 
      Not really interesting for normal applications. 
      Might be useful for implementing rsound on top of other blocking APIs. */
   size_t rsd_pointer (rsound_t *rd);

   /* Aquires how much data can be written to the buffer without blocking */
   size_t rsd_get_avail (rsound_t *rd);

   /* Aquires the latency at the moment for the audio stream. It is measured in bytes. Useful for syncing video and audio. */
   size_t rsd_delay (rsound_t *rd);

   /* Utility for returning latency in milliseconds. */
   size_t rsd_delay_ms (rsound_t *rd);

   /* Returns bytes per sample */
   int rsd_samplesize( rsound_t *rd );

   /* Will sleep until latency of stream reaches maximum allowed latency defined earlier by rsd_set_param - RSD_LATENCY 
      Useful for hard headed blocking I/O design where user defined latency is needed. If rsd_set_param hasn't been set
      with RSD_LATENCY, this function will do nothing. */
   void rsd_delay_wait(rsound_t *rd);


   /* Pauses or unpauses a stream. pause -> enable = 1 
      This function essentially calls on start() and stop(). This behavior might be changed later. */
   int rsd_pause (rsound_t *rd, int enable);

   /* Frees an rsound_t struct. Make sure that the stream is properly closed down with rsd_stop() before calling rsd_free(). */
   int rsd_free (rsound_t *rd);

#ifdef __cplusplus
}
#endif

#endif



