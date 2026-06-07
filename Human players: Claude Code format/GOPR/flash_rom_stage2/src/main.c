#include "bootrom_api.h"
#include "terminal.h"

void main(void) {
    game_terminal();
    return;
}

void _start(void) {
    main();
    while (1);
}