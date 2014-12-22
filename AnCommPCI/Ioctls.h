// IOCTLS.H -- IOCTL code definitions for fileio driver
// Copyright (C) 2011 by FreeFpga
// All rights reserved

#ifndef IOCTLS_H
#define IOCTLS_H

#ifndef CTL_CODE
	#pragma message("CTL_CODE undefined. Include winioctl.h or wdm.h")
#endif

typedef struct _ReadCmd
{
	unsigned long length;
	unsigned long offset;
}ReadCmd,*PReadCmd;

#define MAX_SEND_LENGTH 0x1000
#define MAX_RECEIVE_LENGTH 0x1000

#define IOCTL_ALLOC_MEM CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x808, \
	METHOD_BUFFERED, \
	FILE_ANY_ACCESS)	


#define IOCTL_SEND_DATA CTL_CODE(\
			FILE_DEVICE_UNKNOWN, \
			0x809, \
			METHOD_BUFFERED, \
			FILE_ANY_ACCESS)			


#define IOCTL_RECEIVE_DATA CTL_CODE(\
			FILE_DEVICE_UNKNOWN, \
			0x80A, \
			METHOD_BUFFERED, \
			FILE_ANY_ACCESS)	


#define IOCTL_BEGIN_RECEIVE_DATA CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x80B, \
	METHOD_BUFFERED, \
	FILE_ANY_ACCESS)	



#endif