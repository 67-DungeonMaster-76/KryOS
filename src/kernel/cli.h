/*
 * cli.h - Command Line Interface header
 * version 0.0.1
 */

#ifndef CLI_H
#define CLI_H

/* Command buffer size */
#define CMD_BUFFER_SIZE 256

/* Maximum arguments */
#define MAX_ARGS 16

/* Initialize CLI */
void cli_init(void);

/* Run CLI main loop */
void cli_run(void);

/* Process a command */
void cli_process_command(const char *cmd);

#endif /* CLI_H */
