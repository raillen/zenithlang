#include "runtime/c/zenith_rt.h"

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

static void assert_true(const char *name, int condition) {
    if (!condition) {
        fprintf(stderr, "falha no teste %s\n", name);
        exit(1);
    }
}

typedef struct worker_result {
    zt_error_kind kind;
    char message[64];
    char code[32];
    char source_name[64];
    zt_int line;
    zt_int column;
    int saw_error;
} worker_result;

typedef struct worker_args {
    atomic_int *reported_count;
    int total_threads;
    zt_error_kind kind;
    const char *message;
    const char *code;
    const char *source_name;
    zt_int line;
    zt_int column;
    worker_result *result;
} worker_args;

static void copy_text(char *dest, size_t capacity, const char *text) {
    if (capacity == 0) {
        return;
    }

    if (text == NULL) {
        dest[0] = '\0';
        return;
    }

    snprintf(dest, capacity, "%s", text);
}

static void wait_for_other_workers(atomic_int *reported_count, int total_threads) {
    while (atomic_load_explicit(reported_count, memory_order_acquire) < total_threads) {
#ifdef _WIN32
        Sleep(1);
#else
        usleep(1000);
#endif
    }
}

static void run_worker(worker_args *args) {
    const zt_runtime_error_info *error;

    zt_runtime_clear_error();
    zt_runtime_report_error(
        args->kind,
        args->message,
        args->code,
        zt_runtime_make_span(args->source_name, args->line, args->column));

    atomic_fetch_add_explicit(args->reported_count, 1, memory_order_acq_rel);
    wait_for_other_workers(args->reported_count, args->total_threads);

    error = zt_runtime_last_error();
    args->result->saw_error = error->has_error ? 1 : 0;
    args->result->kind = error->kind;
    args->result->line = error->span.line;
    args->result->column = error->span.column;
    copy_text(args->result->message, sizeof(args->result->message), error->message);
    copy_text(args->result->code, sizeof(args->result->code), error->code);
    copy_text(args->result->source_name, sizeof(args->result->source_name), error->span.source_name);
}

#ifdef _WIN32
static unsigned __stdcall worker_entry(void *opaque) {
    run_worker((worker_args *)opaque);
    return 0;
}
#else
static void *worker_entry(void *opaque) {
    run_worker((worker_args *)opaque);
    return NULL;
}
#endif

static void test_runtime_error_is_thread_local(void) {
    atomic_int reported_count = 0;
    worker_result first_result = {0};
    worker_result second_result = {0};
    worker_args first_args;
    worker_args second_args;
#ifdef _WIN32
    HANDLE threads[2];
    unsigned thread_id0 = 0;
    unsigned thread_id1 = 0;
#else
    pthread_t threads[2];
#endif

    first_args.reported_count = &reported_count;
    first_args.total_threads = 2;
    first_args.kind = ZT_ERR_IO;
    first_args.message = "worker one";
    first_args.code = "ZT-W1";
    first_args.source_name = "worker_one.zt";
    first_args.line = 11;
    first_args.column = 2;
    first_args.result = &first_result;

    second_args.reported_count = &reported_count;
    second_args.total_threads = 2;
    second_args.kind = ZT_ERR_INDEX;
    second_args.message = "worker two";
    second_args.code = "ZT-W2";
    second_args.source_name = "worker_two.zt";
    second_args.line = 17;
    second_args.column = 5;
    second_args.result = &second_result;

    zt_runtime_clear_error();
    assert_true("main_thread_starts_clear", !zt_runtime_last_error()->has_error);

#ifdef _WIN32
    threads[0] = (HANDLE)_beginthreadex(NULL, 0, worker_entry, &first_args, 0, &thread_id0);
    threads[1] = (HANDLE)_beginthreadex(NULL, 0, worker_entry, &second_args, 0, &thread_id1);
    assert_true("thread0_created", threads[0] != 0);
    assert_true("thread1_created", threads[1] != 0);
    WaitForSingleObject(threads[0], INFINITE);
    WaitForSingleObject(threads[1], INFINITE);
    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
#else
    assert_true("thread0_created", pthread_create(&threads[0], NULL, worker_entry, &first_args) == 0);
    assert_true("thread1_created", pthread_create(&threads[1], NULL, worker_entry, &second_args) == 0);
    assert_true("thread0_join", pthread_join(threads[0], NULL) == 0);
    assert_true("thread1_join", pthread_join(threads[1], NULL) == 0);
#endif

    assert_true("worker1_saw_error", first_result.saw_error);
    assert_true("worker1_kind", first_result.kind == ZT_ERR_IO);
    assert_true("worker1_message", strcmp(first_result.message, "worker one") == 0);
    assert_true("worker1_code", strcmp(first_result.code, "ZT-W1") == 0);
    assert_true("worker1_source", strcmp(first_result.source_name, "worker_one.zt") == 0);
    assert_true("worker1_line", first_result.line == 11);
    assert_true("worker1_column", first_result.column == 2);

    assert_true("worker2_saw_error", second_result.saw_error);
    assert_true("worker2_kind", second_result.kind == ZT_ERR_INDEX);
    assert_true("worker2_message", strcmp(second_result.message, "worker two") == 0);
    assert_true("worker2_code", strcmp(second_result.code, "ZT-W2") == 0);
    assert_true("worker2_source", strcmp(second_result.source_name, "worker_two.zt") == 0);
    assert_true("worker2_line", second_result.line == 17);
    assert_true("worker2_column", second_result.column == 5);

    assert_true("main_thread_stays_clear", !zt_runtime_last_error()->has_error);
}

int main(void) {
    test_runtime_error_is_thread_local();
    printf("Runtime error TLS tests OK\n");
    return 0;
}
