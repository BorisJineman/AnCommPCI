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

typedef struct _DeviceStatus
{
	unsigned long comm;

	unsigned long voltage;
	unsigned long temperature;
	unsigned long x;
	unsigned long y;
	unsigned long z;
}DeviceStatus, *PDeviceStatus;

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

#define IOCTL_GET_DEVICE_STATUS CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x80C, \
	METHOD_BUFFERED, \
	FILE_ANY_ACCESS)	

#define IOCTL_RESET_COMM CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x80D, \
	METHOD_BUFFERED, \
	FILE_ANY_ACCESS)

#define IOCTL_RECEIVE_IGNORE_LEN_DATA CTL_CODE(\
			FILE_DEVICE_UNKNOWN, \
			0x80E, \
			METHOD_BUFFERED, \
			FILE_ANY_ACCESS)	

#define IOCTL_CHECK_RECEIVE_FINISHED CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x80F, \
	METHOD_BUFFERED, \
	FILE_ANY_ACCESS)	

#endif