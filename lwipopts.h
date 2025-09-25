#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Generally you would define your own explicit list of lwIP options
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html)
//
// This example uses a common include to avoid repetition

// --- ELEX 7820 ---

#define NO_SYS                      0
#define LWIP_SOCKET                 1

#include "lwipopts_examples_common.h"

#undef  LWIP_NETCONN
#define LWIP_NETCONN                1

#define LWIP_TIMEVAL_PRIVATE        0

#define TCPIP_MBOX_SIZE                16
#define DEFAULT_TCP_RECVMBOX_SIZE      16
#define DEFAULT_UDP_RECVMBOX_SIZE      16
#define DEFAULT_RAW_RECVMBOX_SIZE      16
#define DEFAULT_ACCEPTMBOX_SIZE        16

#define TCPIP_THREAD_STACKSIZE       4096
#define DEFAULT_THREAD_STACKSIZE     4096
#define TCPIP_THREAD_PRIO            (configMAX_PRIORITIES - 2)

#endif
