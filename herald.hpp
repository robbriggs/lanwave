#ifndef LANWAVE_HERALD
#define LANWAVE_HERALD
/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Herald defines
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
#define HERALD_PORT					12347
#define HERALD_FAST_ANNOUNCES		5
#define HERALD_FAST_ANNOUNCE_DELAY	3
#define HERALD_ANNOUNCE_DELAY		30

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Herald commands
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
#define HCMD_SKIP			(char)0x00
#define HCMD_ANNOUNCE		(char)0x01
#define HCMD_ANNOUNCE_REPLY	(char)0x02

#define HCMD_WHO_IS_MASTER	(char)0x10
#define HCMD_AM_MASTER		(char)0x11
#define HCMD_MASTER_DEATH	(char)0x12

#define HCMD_SELF_CHECK		(char)0x05
#define HCMD_PING			(char)0x06
#define HCMD_PING_REPLY		(char)0x07

#endif