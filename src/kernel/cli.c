/*
 * cli.c - Command Line Interface implementation
 * version 0.0.5
 * Updated with file management commands (ls, touch, cd, pwd, rm, mkdir)
 */

#include "cli.h"
#include "drivers/video/fb_console.h"
#include "drivers/input/keyboard.h"
#include "drivers/video/graphics.h"
#include "demo.h"
#include "utils.h"
#include "drivers/fs/fat32.h"
#include "drivers/fs/ramdisk.h"
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
static const char *cmd_shutdown = "shutdown";
static const char *cmd_ls = "ls";
static const char *cmd_touch = "touch";
static const char *cmd_cd = "cd";
static const char *cmd_pwd = "pwd";
static const char *cmd_rm = "rm";
static const char *cmd_mkdir = "mkdir";
static const char *cmd_crash = "sex";  /* Secret crash command */

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
    fb_print("  shutdown     - Shutdown the system\n");
    fb_print("  clear        - Clear the screen\n");
    fb_print("  test         - Run graphics demo\n");
    fb_print("  ls           - List directory contents\n");
    fb_print("  touch <file> - Create empty file\n");
    fb_print("  cd <dir>     - Change directory\n");
    fb_print("  pwd          - Print working directory\n");
    fb_print("  rm <file>    - Delete file\n");
    fb_print("  mkdir <dir>  - Create directory\n");
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
    demo_rainbow_circle();
}

/*
 * Shutdown command - power off the system
 */
static void cmd_shutdown_exec(void) {
    fb_print("Shutting down...\n");
    fb_flush();
    shutdown();
}

/*
 * Callback for ls command
 */
static void ls_callback(const char *name, uint8_t attr, uint32_t size) {
    if (attr & 0x10) {
        /* Directory */
        fb_print("  <DIR>  ");
    } else {
        /* File */
        fb_print("         ");
    }
    fb_print(name);
    fb_print("\n");
}

/*
 * ls command - list directory contents
 */
static void cmd_ls_exec(const char *args) {
    args = skip_spaces(args);
    if (*args == '\0') {
        args = ".";
    }
    
    if (fat32_list_dir(args, ls_callback) != 0) {
        fb_print("Error: Cannot list directory\n");
    }
}

/*
 * touch command - create empty file
 */
static void cmd_touch_exec(const char *args) {
    args = skip_spaces(args);
    if (*args == '\0') {
        fb_print("Usage: touch <filename>\n");
        return;
    }
    
    if (fat32_create(args) != 0) {
        fb_print("Error: Could not create file\n");
    } else {
        fb_print("Created: ");
        fb_print(args);
        fb_print("\n");
    }
}

/*
 * cd command - change directory
 */
static void cmd_cd_exec(const char *args) {
    args = skip_spaces(args);
    if (*args == '\0') {
        /* Go to root */
        fat32_chdir("/");
        return;
    }
    
    if (fat32_chdir(args) != 0) {
        fb_print("Error: Directory not found\n");
    }
}

/*
 * pwd command - print working directory
 */
static void cmd_pwd_exec(void) {
    fb_print(fat32_getcwd());
    fb_print("\n");
}

/*
 * rm command - delete file
 */
static void cmd_rm_exec(const char *args) {
    args = skip_spaces(args);
    if (*args == '\0') {
        fb_print("Usage: rm <filename>\n");
        return;
    }
    
    if (fat32_delete(args) != 0) {
        fb_print("Error: Could not delete file\n");
    } else {
        fb_print("Deleted: ");
        fb_print(args);
        fb_print("\n");
    }
}

/*
 * mkdir command - create directory
 */
static void cmd_mkdir_exec(const char *args) {
    args = skip_spaces(args);
    if (*args == '\0') {
        fb_print("Usage: mkdir <dirname>\n");
        return;
    }
    
    if (fat32_mkdir(args) != 0) {
        fb_print("Error: Could not create directory\n");
    } else {
        fb_print("Created directory: ");
        fb_print(args);
        fb_print("\n");
    }
}

/*
 * Crash command - intentionally cause a divide by zero exception
 */
static void cmd_crash_exec(void) {
    fb_print("Crashing system...\n");
    fb_flush();
    
    /* Cause divide by zero exception */
    __asm__ __volatile__(
        "xor %%eax, %%eax\n\t"
        "div %%al\n\t"        /* Divide 0 by 0 - causes #DE exception */
        : : : "eax"
    );
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
    
    /* shutdown command */
    if (strcmp(cmd, cmd_shutdown) == 0) {
        cmd_shutdown_exec();
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
    
    /* ls command */
    if (starts_with(cmd, cmd_ls)) {
        if (cmd[2] == ' ' || cmd[2] == '\0') {
            cmd_ls_exec(cmd + 2);
            return;
        }
    }
    
    /* touch command */
    if (starts_with(cmd, cmd_touch)) {
        if (cmd[5] == ' ' || cmd[5] == '\0') {
            cmd_touch_exec(cmd + 5);
            return;
        }
    }
    
    /* cd command */
    if (starts_with(cmd, cmd_cd)) {
        if (cmd[2] == ' ' || cmd[2] == '\0') {
            cmd_cd_exec(cmd + 2);
            return;
        }
    }
    
    /* pwd command */
    if (strcmp(cmd, cmd_pwd) == 0) {
        cmd_pwd_exec();
        return;
    }
    
    /* rm command */
    if (starts_with(cmd, cmd_rm)) {
        if (cmd[2] == ' ' || cmd[2] == '\0') {
            cmd_rm_exec(cmd + 2);
            return;
        }
    }
    
    /* mkdir command */
    if (starts_with(cmd, cmd_mkdir)) {
        if (cmd[5] == ' ' || cmd[5] == '\0') {
            cmd_mkdir_exec(cmd + 5);
            return;
        }
    }
    
    /* crash command (secret) */
    if (strcmp(cmd, cmd_crash) == 0) {
        cmd_crash_exec();
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
    
    fb_print("\nKryOS CLI v0.0.5\n");
    fb_print("Type 'help' for available commands.\n\n");
    
    while (1) {
        /* Show prompt with current directory */
        fb_print(fat32_getcwd());
        fb_print("> ");
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
