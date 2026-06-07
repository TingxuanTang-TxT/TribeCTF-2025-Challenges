#ifndef   __TERMINAL_H
#define  __TERMINAL_H

#include <stdint.h>
#include "system.h"

void bootloader_menu(void);
extern void sp_command(uint32_t val);
extern uint8_t rd_command(void);
extern void go_command(void);
extern void wt_command(uint8_t val);
extern uint32_t get_addr(void);
void bootloader_terminal(void);

#endif