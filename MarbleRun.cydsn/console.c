/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "uart.h"
#include "console.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define ARGMAX 4

static struct console_def *decl_head = NULL;
static char conbuf[UART_BUFLEN];
static unsigned int argnum = 0;
static char *argptr[ARGMAX];

static void chop_params(void) {
    char *buf = conbuf, *last = conbuf;
    argnum = 0;
    for (; *buf; buf++) {
        if (*buf == ' ' && argnum < ARGMAX - 1) {
            *buf = '\0';
            argptr[argnum++] = last;
            last = buf + 1;
        }
    }
    argptr[argnum++] = last;
    for (unsigned int i = argnum; i < ARGMAX; i++) {
        argptr[i] = NULL;
    }
}

const char *console_get_string(unsigned int arg) {
    arg += 1; // don't include command
    assert(arg < argnum);
    char *s = argptr[arg];
    assert(s);
    return s;
}

int console_get_int(unsigned int arg) {
    return atoi(console_get_string(arg));
}

float console_get_float(unsigned int arg) {
    return atof(console_get_string(arg));
}

int console_get_bool_maybe(unsigned int arg) {
    const char *value = console_get_string(arg);
    if (strcmp(value, "true") == 0 || strcmp(value, "on") == 0) {
        return 1;
    } else if (strcmp(value, "false") == 0 || strcmp(value, "off") == 0) {
        return 0;
    } else {
        return -1;
    }
}

void console_register(struct console_def *def) {
    assert(def->argcount <= ARGMAX - 1);
    assert(*def->name);
    def->next = decl_head;
    decl_head = def;
}

void console_perform(const char *input, size_t length) {
    if (length > UART_BUFLEN - 1) {
        length = UART_BUFLEN - 1;
    }
    memcpy(conbuf, input, length);
    conbuf[length] = '\0';

    chop_params();

    const char *command = argptr[0];
    assert(command);
    if (!*command) {
        // no command
        return;
    }
    struct console_def *decl = decl_head;
    while (decl) {
        if (strcmp(command, decl->name) == 0) {
            if (decl->argcount != argnum - 1) {
                uart_send("wrong number of arguments to ");
                uart_send(decl->name);
                uart_send(": got ");
                snprintf(conbuf, UART_BUFLEN, "%u", argnum - 1);
                uart_send(conbuf);
                uart_send(" instead of ");
                snprintf(conbuf, UART_BUFLEN, "%u", decl->argcount);
                uart_send(conbuf);
                uart_send("\r\n");
                return;
            }
            decl->cb(decl->param);
            return;
        }
        decl = decl->next;
    }
    uart_send("no such command ");
    uart_send(command);
    uart_send("\r\n");
}

// default commands

static void cmd_help(void *p) {
    (void) p;
    uart_send("commands:\r\n");
    struct console_def *decl = decl_head;
    char padding[31];
    memset(padding, ' ', 31);
    while (decl) {
        uart_send("  ");
        uart_send(decl->name);
        int padn = 30 - strlen(decl->name);
        if (padn < 0) {
            padn = 0;
        }
        decl = decl->next;
        if (decl) {
            padding[padn] = '\0';
            uart_send(padding);
            padding[padn] = ' ';
            uart_send(decl->name);
            decl = decl->next;
        }
        uart_send("\r\n");
    }
}
static struct console_def def_help = { .name = "help", .argcount = 0, .cb = cmd_help };

static void cmd_echo(void *p) {
    (void) p;
    uart_send("[echo] ");
    uart_send(console_get_string(0));
    uart_send("\r\n");
}
static struct console_def def_echo = { .name = "echo", .argcount = 1, .cb = cmd_echo };

void initialize_console(void) {
    console_register(&def_help);
    console_register(&def_echo);
    uart_send("console ok\r\n");
}

/* [] END OF FILE */
