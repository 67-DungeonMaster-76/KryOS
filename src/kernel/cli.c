/*
 * cli.c - Command Line Interface implementation
 * version 0.0.3
 * Updated for framebuffer console
 */

#include "cli.h"
#include "fb_console.h"
#include "keyboard.h"
#include "graphics.h"
#include "demo.h"
#include "stdint.h"

/* Command buffer */
static char cmd_buffer[CMD_BUFFER_SIZE];
static int cmd_pos = 0;

/* Prompt string */
static const char *prompt = "kryos> ";

/* Command names */
static const char *cmd_echo = "echo";
static const char *cmd_help = "help";
static const char *cmd_halt = "halt";
static const char *cmd_clear = "clear";
static const char *cmd_test = "test";

/* Compare two strings */
static int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

/* Compare string with prefix */
static int starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*str != *prefix) {
            return 0;
        }
        str++;
        prefix++;
    }
    return 1;
}

/* Skip whitespace */
static const char *skip_spaces(const char *str) {
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    return str;
}

/*
 * Echo command - print arguments
 */
static void cmd_echo_exec(const char *args) {
    args = skip_spaces(args);
    fb_print(args);
    fb_putchar('\n');
}

/*
 * Help command - show available commands
 */
static void cmd_help_exec(void) {
    fb_print("Available commands:\n");
    fb_print("  echo <text>  - Print text to screen\n");
    fb_print("  help         - Show this help message\n");
    fb_print("  halt         - Halt the system\n");
    fb_print("  clear        - Clear the screen\n");
    fb_print("  test         - Run graphics demo\n");
}

/*
 * Halt command - stop the system
 */
static void cmd_halt_exec(void) {
    fb_print("System halting...\n");
    __asm__ __volatile__("cli; hlt");
}

/*
 * Clear command - clear screen
 */
static void cmd_clear_exec(void) {
    fb_console_clear();
}

/*
 * Test command - run graphics demo
 */
static void cmd_test_exec(void) {
    fb_print("Starting graphics demo...\n");
    fb_print("Press any key to return to CLI.\n");
    /* wait(500); */
    demo_rainbow_circle();
}

/*
 * Process a command
 */
void cli_process_command(const char *cmd) {
    cmd = skip_spaces(cmd);
    
    /* Empty command */
    if (*cmd == '\0') {
        return;
    }
    
    /* Parse command using switch-like approach with string comparison */
    
    /* echo command */
    if (starts_with(cmd, cmd_echo)) {
        if (cmd[4] == ' ' || cmd[4] == '\0') {
            cmd_echo_exec(cmd + 4);
            return;
        }
    }
    
    /* help command */
    if (strcmp(cmd, cmd_help) == 0) {
        cmd_help_exec();
        return;
    }
    
    /* halt command */
    if (strcmp(cmd, cmd_halt) == 0) {
        cmd_halt_exec();
        return;
    }
    
    /* clear command */
    if (strcmp(cmd, cmd_clear) == 0) {
        cmd_clear_exec();
        return;
    }
    
    /* test command */
    if (strcmp(cmd, cmd_test) == 0) {
        cmd_test_exec();
        return;
    }
    
    /* Unknown command */
    fb_print("Unknown command: ");
    fb_print(cmd);
    fb_print("\nType 'help' for available commands.\n");
}

/*
 * Initialize CLI
 */
void cli_init(void) {
    cmd_pos = 0;
    cmd_buffer[0] = '\0';
}

/*
 * Run CLI main loop
 */
void cli_run(void) {
    char c;
    
    fb_print("\nKryOS CLI v0.0.3\n");
    fb_print("Type 'help' for available commands.\n\n");
    
    while (1) {
        /* Show prompt */
        fb_print(prompt);
        cmd_pos = 0;
        
        /* Read command line */
        while (1) {
            c = keyboard_getchar();
            
            if (c == '\n') {
                /* Enter pressed - execute command */
                fb_putchar('\n');
                cmd_buffer[cmd_pos] = '\0';
                cli_process_command(cmd_buffer);
                break;
            } else if (c == '\b') {
                /* Backspace */
                if (cmd_pos > 0) {
                    cmd_pos--;
                    fb_putchar('\b');
                    fb_flush();  /* Flush so user sees it immediately */
                }
            } else if (c >= ' ' && cmd_pos < CMD_BUFFER_SIZE - 1) {
                /* Regular character */
                cmd_buffer[cmd_pos++] = c;
                fb_putchar(c);
                fb_flush();  /* Flush so user sees it immediately */
            }
        }
    }
}
