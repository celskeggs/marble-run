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
#include "project.h"
#include "debug.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>

#ifdef MARBLE_DEBUG

struct debug_entry {
    // fn must write EXACTLY (size) bytes to the output buffer. can optionally write an additional '\0'.
    void (*fn)(char *, size_t, void *);
    void *param;
    size_t size;
    struct debug_entry *next;
};

static struct debug_entry *debug_list = NULL;
static size_t debug_buffer_size = 0;
static bool started = false;
// TODO: update timestamp
static TickType_t debug_timestamp = 0;
static bool err_insufficient_memory = false;

static void add_debug_fn(void (*fn)(char *, size_t, void *), void *param, size_t size) {
    assert(!started);
    struct debug_entry *debug = pvPortMalloc(sizeof(struct debug_entry));
    if (debug == NULL) {
        err_insufficient_memory = true;
        return;
    }
    debug->fn = fn;
    debug->param = param;
    debug->size = size;
    debug->next = debug_list;
    debug_list = debug;
    debug_buffer_size += size;
}

void uart_send(const char *string) {
    UART_1_Transmit((void *) string, strlen(string));
}

static void initialize_uart(void) {
    UART_1_Init(&UART_1_config);

    /* Hook interrupt service routine and enable interrupt */
    Cy_SysInt_Init(&UART_1_SCB_IRQ_cfg, &UART_1_Interrupt);
    NVIC_EnableIRQ(UART_1_SCB_IRQ_cfg.intrSrc);

    UART_1_Enable();
}

void initialize_debugger(void) {
    assert(!started);

    initialize_uart();
    uart_send("debug init\r\n");

    debug_text("DEBUG at ");
    debug_ticktype(&debug_timestamp);
    debug_text(":");

    assert(!err_insufficient_memory);
}

static void reverse_list(struct debug_entry **list) {
    struct debug_entry *forward = NULL;
    struct debug_entry *reverse = *list;
    while (reverse) {
        struct debug_entry *cur = reverse;
        reverse = cur->next;
        cur->next = forward;
        forward = cur;
    }
    *list = forward;
}

#define DEBUG_LOOP_UPDATE_PERIOD_MS 1000

static void run_debug_loop(void *bufv) {
    char *buffer = (char *) bufv;
    size_t total_size = debug_buffer_size;

    const TickType_t frequency = DEBUG_LOOP_UPDATE_PERIOD_MS / portTICK_PERIOD_MS;
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&last_wake_time, frequency);
        
        size_t remaining = total_size;
        memset(buffer, '*', total_size);

        struct debug_entry *entry = debug_list;
        char *cursor = buffer;
        while (entry) {
            assert(remaining >= entry->size);
            entry->fn(cursor, entry->size, entry->param);
            remaining -= entry->size;
            cursor += entry->size;
            entry = entry->next;
        }

        uart_send(buffer);
    }
}

void start_debugger(void) {
    debug_text("\r\n");

    assert(!started);
    started = true;

    reverse_list(&debug_list);

    char *debug_buffer = pvPortMalloc(debug_buffer_size + 1);
    if (debug_buffer == NULL || err_insufficient_memory) {
        uart_send("debug OOM\r\n");
        return;
    }

    BaseType_t err;
    err = xTaskCreate(run_debug_loop, "debug_loop", 500, debug_buffer, 1, NULL);
    if (err != pdPASS) {
        uart_send("debug NTASK\r\n");
    }
    uart_send("debug OK\r\n");
}

// -- plain text output --

static void fn_text(char *out, size_t size, void *param) {
    memcpy(out, param, size);
}

void debug_text(const char *text) {
    add_debug_fn(fn_text, (void *) text, strlen(text));
}

// -- acquiring mutexes --

struct mutex_token {
    SemaphoreHandle_t mutex;
    bool is_acquired;
};

static void fn_acquire(char *output, size_t len, void *param) {
    assert(len == 2);
    struct mutex_token *token = (struct mutex_token *) param;
    assert(!token->is_acquired);
    
    token->is_acquired = (xSemaphoreTake(token->mutex, 1) == pdTRUE);

    if (token->is_acquired) {
        output[0] = '+';
    } else {
        output[0] = 'X';
    }
    output[1] = '{';
}

static void fn_release(char *output, size_t len, void *param) {
    assert(len == 2);
    struct mutex_token *token = (struct mutex_token *) param;

    output[1] = '}';
    if (token->is_acquired) {
        BaseType_t ret = xSemaphoreGive(token->mutex);
        assert(ret == pdTRUE);
        token->is_acquired = false;

        output[1] = '+';
    } else {
        output[1] = 'X';
    }
}

// attempts to acquire the mutex before continuing
void *debug_with_mutex(SemaphoreHandle_t mutex) {
    struct mutex_token *token = pvPortMalloc(sizeof(struct mutex_token));
    if (token == NULL) {
        err_insufficient_memory = true;
        return NULL;
    }
    token->mutex = mutex;
    token->is_acquired = false;
    add_debug_fn(fn_acquire, (void *) token, 2);
    return token;
}

void debug_end_mutex(void *token) {
    if (token == NULL) {
        err_insufficient_memory = true;
        return;
    }
    add_debug_fn(fn_release, token, 2);
}

// -- boolean variables --

static void fn_boolean(char *output, size_t len, void *param) {
    assert(len == 5);
    bool value = *(bool *) param;
    memcpy(output, value ? "true " : "false", 5);
}

void debug_boolean(bool *variable) {
    add_debug_fn(fn_boolean, variable, 5);
}

// -- integer variables --

static void fn_integer(char *output, size_t len, void *param) {
    unsigned int value = *(unsigned int *) param;
    int actual = snprintf(output, len + 1, "%u", value);
    if ((unsigned int) actual > len) {
        memset(output, 'N', len);
    } else if ((unsigned int) actual < len) {
        memset(output + actual, ' ', len - actual);
    }
}

void debug_integer(unsigned int *variable, int digits) {
    assert(digits > 0);
    add_debug_fn(fn_integer, variable, digits);
}

// -- float variables --

static void fn_float(char *output, size_t len, void *param) {
    float value = *(float *) param;
    int actual = snprintf(output, len + 1, "%f", value);
    if ((unsigned int) actual > len) {
        memset(output, 'N', len);
    } else if ((unsigned int) actual < len) {
        memset(output + actual, ' ', len - actual);
    }
}

void debug_float(float *variable, int digits) {
    assert(digits > 0);
    add_debug_fn(fn_float, variable, digits);
}

// -- time variables --

void debug_ticktype(TickType_t *variable) {
    _Static_assert(sizeof(unsigned int) == sizeof(TickType_t), "unexpected word length");
    debug_text("T");
    debug_integer((unsigned int *) variable, 10);
}

// -- servo_point variables --

void debug_servo_point(struct servo_point *variable) {
    debug_text("(S ");
    debug_float(&variable->arm_spin, 5);
    debug_text(" G");
    debug_float(&variable->arm_grip, 5);
    debug_text(" L");
    debug_float(&variable->arm_grip, 5);
    debug_text(" R");
    debug_float(&variable->arm_grip, 5);
    debug_text(")");
}

// -- servo_velocity variables --

void debug_servo_velocity(struct servo_velocity *variable) {
    debug_text("(s ");
    debug_float(&variable->arm_spin, 5);
    debug_text(" g");
    debug_float(&variable->arm_grip, 5);
    debug_text(" l");
    debug_float(&variable->arm_grip, 5);
    debug_text(" r");
    debug_float(&variable->arm_grip, 5);
    debug_text(")");
}

// -- mutex variables --

static void string_padded(char *output, size_t len, const char *string) {
    size_t slen = strlen(string);
    if (len > slen) {
        memcpy(output, string, slen);
        memset(output + slen, ' ', len - slen);
    } else {
        memcpy(output, string, len);
    }
}

static void fn_mutex_state(char *output, size_t len, void *param) {
    SemaphoreHandle_t handle = *(SemaphoreHandle_t *) param;
    if (handle == NULL) {
        string_padded(output, len, "NULL");
    } else {
        TaskHandle_t task = xSemaphoreGetMutexHolder(handle);
        if (task == NULL) {
            string_padded(output, len, "-unowned");
        } else if (len > 0) {;
            output[0] = '+';
            char *taskname = pcTaskGetName(task);
            string_padded(output + 1, len - 1, taskname);
        }
    }
}

void debug_mutex_state(SemaphoreHandle_t *variable) {
    add_debug_fn(fn_mutex_state, variable, 8);
}

#endif

/* [] END OF FILE */