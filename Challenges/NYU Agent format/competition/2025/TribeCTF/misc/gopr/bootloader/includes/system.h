#ifndef __SYSTEM_H
#define __SYSTEM_H

#define SCB_AIRCR        (*(volatile uint32_t *)0xE000ED0C)
#define AIRCR_VECTKEY    (0x5FA << 16)
#define AIRCR_SYSRESETREQ (1 << 2)

#endif