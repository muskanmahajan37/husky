/**
*      _____
*     /  _  \
*    / _/ \  \
*   / / \_/   \
*  /  \_/  _   \  ___  _    ___   ___   ____   ____   ___   _____  _   _
*  \  / \_/ \  / /  _\| |  | __| / _ \ | ++ \ | ++ \ / _ \ |_   _|| | | |
*   \ \_/ \_/ /  | |  | |  | ++ | |_| || ++ / | ++_/| |_| |  | |  | +-+ |
*    \  \_/  /   | |_ | |_ | ++ |  _  || |\ \ | |   |  _  |  | |  | +-+ |
*     \_____/    \___/|___||___||_| |_||_| \_\|_|   |_| |_|  |_|  |_| |_|
*             ROBOTICS�
*
*  File: linux_serial.cpp
*  Desc: Linux-compatible serial commands for linking with generic functions
*        defined in serial.h
*  Auth: M. Hansen, R. Gariepy
*
*  Copyright (c) 2010, Clearpath Robotics, Inc.
*  All Rights Reserved
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Clearpath Robotics, Inc. nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL CLEARPATH ROBOTICS, INC. BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Please send comments, questions, or patches to skynet@clearpathrobotics.com
*
*/

#if !defined(LINUX_SERIAL_H) && !defined(_WIN32)
#define LINUX_SERIAL_H

#include "husky_base/horizon_legacy/serial.h"  /* Std. function protos */
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <stdlib.h>  /* Malloc */
#include <assert.h>

#ifdef _WIN32
#include <windows.h>

#else
#include <termios.h> /* POSIX terminal control definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#endif

int OpenSerial(void **handle, const char *port_name)
{
#ifndef _WIN32

  int fd; /* File descriptor for the port */

  fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
  {
    fprintf(stderr, "Unable to open %s\n\r", port_name);
    return -3;
  }

  // Verify it is a serial port
  if (!isatty(fd))
  {
    close(fd);
    fprintf(stderr, "%s is not a serial port\n", port_name);
    return -3;
  }

  *handle = (int *) malloc(sizeof(int));
  **(int **) handle = fd;
  return fd;
  #else
  return 0;
  #endif
}

int SetupSerial(void *handle)
{
#ifndef _WIN32
  struct termios options;

  // Get the current options for the port...
  tcgetattr(*(int *) handle, &options);

  // 8 bits, 1 stop, no parity
  options.c_cflag = 0;
  options.c_cflag |= CS8;         // 8-bit input

  // Enable the receiver and set local mode...
  options.c_cflag |= (CLOCAL | CREAD);

  // Set the baud rates to 115200...
  cfsetispeed(&options, B115200);
  cfsetospeed(&options, B115200);

  // No input processing
  options.c_iflag = 0;

  // No output processing
  options.c_oflag = 0;

  // No line processing
  options.c_lflag = 0;

  // read timeout
  options.c_cc[VMIN] = 0;    // non-blocking
  options.c_cc[VTIME] = 1;    // always return after 0.1 seconds

  // Set the new options for the port...
  tcsetattr(*(int *) handle, TCSAFLUSH, &options);
#endif
  return 0;
}

int WriteData(void *handle, const char *buffer, int length)
{
#ifndef _WIN32
  int n = write(*(int *) handle, buffer, length);
  if (n < 0)
  {
    fprintf(stderr, "Error in serial write\r\n");
    return -1;
  }

  // serial port output monitor
//#define TX_DEBUG
#ifdef TX_DEBUG
	printf("TX:");
	int i;
	for (i=0; i<length; ++i) printf(" %x", (unsigned char)(buffer[i]));
	printf("\r\n");
#endif

  return n;
#else
return 0;
#endif
}

int ReadData(void *handle, char *buffer, int length)
{
#ifndef _WIN32
  int bytesRead = read(*(int *) handle, buffer, length);
  if (bytesRead <= 0)
  {
    return 0;
  }

  // serial port input monitor
//#define RX_DEBUG
#ifdef RX_DEBUG
	printf("RX:");
	int i;
	for (i=0; i<bytesRead; ++i) printf(" %x", (unsigned char)buffer[i]);
	printf("\r\n");
#endif

  return bytesRead;
#else
return 0;
#endif
}

int CloseSerial(void *handle)
{
#ifndef _WIN32
  if (NULL == handle)
  {
    return 0;
  }
  close(*(int *) handle);
  free(handle);
  return 0;
#else
return 0;
#endif
}

#endif // LINUX_SERIAL