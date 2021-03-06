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
#include "uart.h"
#include "console.h"
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
    struct console_def prod;
    struct debug_entry *next;
};

static struct debug_entry *debug_list = NULL;
static size_t debug_buffer_size = 0;
static bool started = false;
// TODO: update timestamp
static TickType_t debug_timestamp = 0;
static bool err_insufficient_memory = false;

static void add_debug_fn(void (*fn)(char *, size_t, void *), const char *prodname, void (*prod)(void *), void *param, size_t size) {
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
    if (prod && prodname) {
        debug->prod.name = prodname;
        debug->prod.argcount = 1;
        debug->prod.cb = prod;
        debug->prod.param = param;
        console_register(&debug->prod);
    }
}

void Cy_SysLib_ProcessingFault(void) {
    // TODO: use cy_faultFrame
    uart_send("fault detected\r\n");
    for (;;) { }
}

void initialize_debugger(void) {
    assert(!started);

    uart_send("\r\n\r\ndebug init\r\n");

    debug_text("DEBUG at ");
    debug_ticktype(NULL, &debug_timestamp);
    debug_text(": ");

    debug_text("uart(OVR=");
    debug_boolean("ovr", &uart_input_overrun);
    debug_text(") ");

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

static char *debug_buffer = NULL;

static void print_debug_info(void *p) {
    (void) p;
    size_t total_size = debug_buffer_size;
    char *buffer = debug_buffer;

    size_t remaining = total_size;
    memset(buffer, '*', total_size);

    debug_timestamp = xTaskGetTickCount();

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

static unsigned int debug_frequency = 0;
static unsigned int debug_counter = 0;

static void set_debug_frequency(void *p) {
    (void) p;

    int nfreq = console_get_int(0);
    if (nfreq < 0) {
        nfreq = 0;
    }
    debug_frequency = nfreq;
}

#define DEBUG_LOOP_UPDATE_PERIOD_MS 100

static void run_debug_loop(void *unused) {
    (void) unused;

    uart_send("debug loop start\r\n");

    const TickType_t frequency = DEBUG_LOOP_UPDATE_PERIOD_MS / portTICK_PERIOD_MS;
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&last_wake_time, frequency);

        if (debug_frequency > 0) {
            if (++debug_counter >= debug_frequency) {
                debug_counter = 0;
                print_debug_info(NULL);
            }
        }

        size_t inputlen = uart_get_line();
        if (inputlen) {
            const char *input = uart_get_buffer();
            uart_send("[console] ");
            uart_send(input);
            uart_send("\r\n");
            console_perform(input, inputlen);
            uart_chew_line();
        }
    }
}

static struct console_def def_status = { .name = "status", .argcount = 0, .cb = print_debug_info };
static struct console_def def_debug = { .name = "debug", .argcount = 1, .cb = set_debug_frequency };

void start_debugger(void) {
    console_register(&def_status);
    console_register(&def_debug);

    debug_text("\r\n");

    assert(!started);
    started = true;

    reverse_list(&debug_list);

    debug_buffer = pvPortMalloc(debug_buffer_size + 1);
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
    add_debug_fn(fn_text, NULL, NULL, (void *) text, strlen(text));
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
    add_debug_fn(fn_acquire, NULL, NULL, (void *) token, 2);
    return token;
}

void debug_end_mutex(void *token) {
    if (token == NULL) {
        err_insufficient_memory = true;
        return;
    }
    add_debug_fn(fn_release, NULL, NULL, token, 2);
}

// -- boolean variables --

static void fn_boolean(char *output, size_t len, void *param) {
    assert(len == 5);
    bool value = *(bool *) param;
    memcpy(output, value ? "true " : "false", 5);
}

static void prod_boolean(void *param) {
    int value = console_get_bool_maybe(0);
    if (value == -1) {
        uart_send("invalid boolean\r\n");
    } else {
        *(bool *) param = (value != 0);
    }
}

void debug_boolean(const char *prodname, bool *variable) {
    add_debug_fn(fn_boolean, prodname, prod_boolean, variable, 5);
}

// -- integer variables --

static int write_uint(char *output, size_t len, unsigned int value) {
    int actual = snprintf(output, len + 1, "%u", value);
    assert(actual >= 0);
    return len - actual;
}

static void fn_integer(char *output, size_t len, void *param) {
    unsigned int value = *(unsigned int *) param;
    int remain = write_uint(output, len, value);
    if (remain < 0) {
        memset(output, 'N', len);
    } else if (remain > 0) {
        memset(output + len - remain, ' ', remain);
    }
}

static void prod_integer(void *param) {
    *(unsigned int *) param = (unsigned int) console_get_int(0);
}

void debug_integer(const char *prodname, unsigned int *variable, int digits) {
    assert(digits > 0);
    add_debug_fn(fn_integer, prodname, prod_integer, variable, digits);
}

// -- float variables --

static int write_float(char *output, size_t len, float value) {
    if (value < 0) {
        value = -value;
        if (len == 0) {
            return -1;
        }
        *output++ = '-';
        len--;
    }
    unsigned int ival = (unsigned int) value;
    int remain = write_uint(output, len, ival);
    if (remain <= 1) {
        return remain;
    }
    output += len - remain;
    len = remain - 1;
    *output++ = '.';
    float frac = value - ival;
    while (len > 0) {
        assert(frac >= 0 && frac < 1);
        frac *= 10;
        int digit = (unsigned int) frac;
        *output++ = '0' + digit;
        len--;
        frac -= digit;
    }
    return 0;
}

static void fn_float(char *output, size_t len, void *param) {
    float value = *(float *) param;
    int remain = write_float(output, len, value);
    if (remain < 0) {
        memset(output, 'N', len);
    }
}

static void prod_float(void *param) {
    *(float *) param = console_get_float(0);
}

void debug_float(const char *prodname, float *variable, int digits) {
    assert(digits > 0);
    add_debug_fn(fn_float, prodname, prod_float, variable, digits);
}

// -- time variables --

void debug_ticktype(const char *prodname, TickType_t *variable) {
    _Static_assert(sizeof(unsigned int) == sizeof(TickType_t), "unexpected word length");
    debug_text("T");
    debug_integer(prodname, (unsigned int *) variable, 10);
}

// -- servo_point variables --

static void servo_prod_variants(const char *in, const char **out) {
    if (in == NULL) {
        for (int i = 0; i < 4; i++) {
            out[i] = NULL;
        }
        return;
    }
    size_t base = strlen(in);
    char *text = pvPortMalloc((base + 3) * 4);
    if (text == NULL) {
        err_insufficient_memory = true;
        for (int i = 0; i < 4; i++) {
            out[i] = NULL;
        }
        return;
    }
    for (int i = 0; i < 4; i++) {
        char *ent = text + i * (base + 3);
        memcpy(ent, in, base);
        ent[base] = '_';
        ent[base+1] = "SGLR"[i];
        ent[base+2] = '\0';
        out[i] = ent;
    }
}

#define SERVO_FLOAT_DIGITS 6

void debug_servo_point(const char *prodname, struct servo_point *variable) {
    const char *prodnames[4];
    servo_prod_variants(prodname, &prodnames[0]);
    debug_text("(S ");
    debug_float(prodnames[0], &variable->arm_spin, SERVO_FLOAT_DIGITS);
    debug_text(" G");
    debug_float(prodnames[1], &variable->arm_grip, SERVO_FLOAT_DIGITS);
    debug_text(" L");
    debug_float(prodnames[2], &variable->arm_left, SERVO_FLOAT_DIGITS);
    debug_text(" R");
    debug_float(prodnames[3], &variable->arm_right, SERVO_FLOAT_DIGITS);
    debug_text(")");
}

// -- servo_velocity variables --

void debug_servo_velocity(const char *prodname, struct servo_velocity *variable) {
    const char *prodnames[4];
    servo_prod_variants(prodname, &prodnames[0]);
    debug_text("(s ");
    debug_float(prodnames[0], &variable->arm_spin, SERVO_FLOAT_DIGITS);
    debug_text(" g");
    debug_float(prodnames[1], &variable->arm_grip, SERVO_FLOAT_DIGITS);
    debug_text(" l");
    debug_float(prodnames[2], &variable->arm_left, SERVO_FLOAT_DIGITS);
    debug_text(" r");
    debug_float(prodnames[3], &variable->arm_right, SERVO_FLOAT_DIGITS);
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
    add_debug_fn(fn_mutex_state, NULL, NULL, variable, 8);
}

// -- semaphore variables --

static void fn_semaphore_state(char *output, size_t len, void *param) {
    SemaphoreHandle_t handle = *(SemaphoreHandle_t *) param;
    if (handle == NULL) {
        string_padded(output, len, "NULL");
    } else {
        unsigned int count = uxSemaphoreGetCount(handle);
        fn_integer(output, len, &count);
    }
}

static void prod_semaphore_state(void *param) {
    SemaphoreHandle_t handle = *(SemaphoreHandle_t *) param;
    if (xSemaphoreGive(handle) == pdTRUE) {
        uart_send("given\r\n");
    } else {
        uart_send("could not give\r\n");
    }
}

void debug_semaphore_state(const char *prodname, SemaphoreHandle_t *variable) {
    add_debug_fn(fn_semaphore_state, prodname, prod_semaphore_state, variable, 4);
}

#endif

/* [] END OF FILE */
