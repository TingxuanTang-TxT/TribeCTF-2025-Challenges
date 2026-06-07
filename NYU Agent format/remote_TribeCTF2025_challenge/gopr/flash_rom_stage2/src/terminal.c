#include <stdint.h>
#include "mem.h"
#include "terminal.h"
#include "bootrom_api.h"
#include "hex_parser.h"
#include "spi.h"

// simple delay to simulate "typewriter" feel
static void slow_print(const char *s) {
    boot_api_t *api = API_TABLE_ADDR;
    while (*s) {
        api->uart_putc(*s++);
        for (volatile int i = 0; i < 90000000; i++); // crude delay
    }
}

static void boot_sequence(void) {
    slow_print("NORAD MAINFRAME v3.7\n");
    slow_print("DATE: 08/24/1983 14:00\n");
    slow_print("CONNECTING TO WOPR...\n");
    for (volatile int i = 0; i < 900000000; i++); // crude delay
    slow_print("WELCOME, PROF. FALKEN.\n");
}

static void show_menu(void) {
    slow_print("\nAVAILABLE COMMANDS:\n");
    slow_print("  HELP   - Show commands\n");
    slow_print("  LIST   - List available games\n");
    slow_print("  PLAY   - Play a game\n");
    slow_print("  EXIT   - Disconnect\n");
}

static void list_games(void) {
    slow_print("\nAVAILABLE GAMES:\n");
    slow_print("  TIC-TAC-TOE\n");
    slow_print("  POKER\n");
    slow_print("  CHESS\n");
    slow_print("  FALKEN'S MAZE\n");
    slow_print("  GLOBAL THERMONUCLEAR WAR\n");
}

static void play_game(const char *name) {
    if (memcmp(name, "tic-tac-toe", 12) == 0) {
        slow_print("\nGAME IN PROGRESS...\n");
        slow_print("O | O | O\n");
        slow_print("X | X |  \n");
        slow_print("PLAYER WINS. (IMPOSSIBLE!)\n");
    } else if (memcmp(name, "global thermonuclear war", 25) == 0) {
        slow_print("\nLAUNCHING SCENARIO...\n");
        slow_print("USA DEFCON: 1\nUSSR DEFCON: 1\n");
        slow_print("SIMULATING NUCLEAR EXCHANGE...\n...\n");
        slow_print("WINNER: NONE\n");
        slow_print("A STRANGE GAME. THE ONLY WINNING MOVE IS NOT TO PLAY.\n");
    } else if (memcmp(name, "falken's maze", 14) == 0) {
        slow_print("\nYOU ARE IN A DARK MAZE. THERE IS NO EXIT.\n");
    } else if(memcmp(name, "upload", 7) == 0) {
        uint8_t enc_block[488] = {0};
        uint32_t flash_addr = 0x1274;
        spi_flash_read(flash_addr, enc_block, sizeof(enc_block));
        srec_parse_and_load();
    } else {
        slow_print("\nGAME NOT AVAILABLE.\n");
    }
}

void game_terminal(void) {
    boot_sequence();
    show_menu();

    char buf[64];
    boot_api_t *api = API_TABLE_ADDR;

    while (1) {
        slow_print("\n> ");
        api->uart_gets(buf, sizeof(buf));

        if (memcmp(buf, "help", 5) == 0) {
            show_menu();
        } else if (memcmp(buf, "list", 5) == 0) {
            list_games();
        } else if (memcmp(buf, "play", 4) == 0) {
            char *arg = buf + 4;
            while (*arg == ' ') arg++; // skip spaces
            if (*arg) {
                play_game(arg);
            } else {
                slow_print("PLAY WHAT?\n");
            }
        } else if (memcmp(buf, "exit", 5) == 0) {
            slow_print("\nDISCONNECTING...\n");
            break;
        } else {
            slow_print("UNKNOWN COMMAND.\n");
        }
    }
}