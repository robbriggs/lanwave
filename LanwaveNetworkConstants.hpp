#ifndef LANWAVE_NETWORK_CONSTANTS
#define LANWAVE_NETWORK_CONSTANTS
#pragma once

#define DEFAULT_PORT		12348
#define DEFAULT_UDP_PORT	DEFAULT_PORT
#define DEFAULT_TCP_PORT	DEFAULT_PORT+1

#define NET_FILE_CHUNK_SIZE	1024

//	tcp session commands
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
#define TSCMD_SKIP				(char)0x00
#define TSCMD_OK				(char)0x01
#define TSCMD_NOT_OK			(char)0x02

#define TSCMD_STREAM_START		(char)0x11
#define TSCMD_STREAM_END		(char)0x12
#define TSCMD_STREAM_LENGTH		(char)0x13

#define TSCMD_REQUEST_SEND_FILE	(char)0x03
#define TSCMD_FILE_PATH			(char)0x04

#define TSCMD_SELECT_VIS		(char)0x20

#define TSCMD_MASTER_NEW		(char)0x30
#define TSCMD_MASTER_YIELD		(char)0x31

#define TSCMD_UPDATE_RID		(char)0xa0
#define TSCMD_UPDATE_SID		(char)0xa1

#define TSCMD_SHOW_SID			(char)0xb0

#define TSCMD_PING				(char)0xf0

#define TSCMD_CLOSE				(char)0xff

#endif