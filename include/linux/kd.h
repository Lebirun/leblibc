#ifndef _LINUX_KD_H
#define _LINUX_KD_H

#define KDMKTONE 0x4B30
#define KDGETLED 0x4B31
#define KDSETLED 0x4B32
#define KIOCSOUND 0x4B2F
#define KDGKBTYPE 0x4B33
#define KDSETMODE 0x4B3A
#define KDGETMODE 0x4B3B
#define KDGKBMODE 0x4B44
#define KDSKBMODE 0x4B45
#define KDGKBLED 0x4B64
#define KDSKBLED 0x4B65

#define KD_TEXT 0x00
#define KD_GRAPHICS 0x01
#define KB_101 0x02
#define K_RAW 0x00
#define K_XLATE 0x01
#define K_MEDIUMRAW 0x02
#define K_UNICODE 0x03
#define K_OFF 0x04

#endif
