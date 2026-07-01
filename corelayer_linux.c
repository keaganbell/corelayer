#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

/*== Linux Code =========================================*/
typedef enum { Linux_Object_Kind_None,
    Linux_Object_Kind_Thread,
    Linux_Object_Kind_Mutex,
    Linux_Object_Kind_RW_Mutex,
    Linux_Object_Kind_Condition,
    Linux_Object_Kind_Barrier,
} Linux_Object_Kind;

typedef struct Linux_Object Linux_Object;
struct Linux_Object {
    Linux_Object *next;
    Linux_Object_Kind kind;
    union {
        struct {
            Thread_Entry_Point_Func *func;
            void *params;
            pthread_t handle;
        } thread;
        pthread_mutex_t     mutex;
        pthread_rwlock_t    rw_mutex;
        pthread_cond_t      cv;
        pthread_barrier_t   barrier;
    };
};

/* pool allocation of sync objects */
function Linux_Object *linux_object_alloc(Linux_Object_Kind kind);
function void linux_object_free(Linux_Object *obj);

global struct {
    Arena *arena;

    /* info */
    System_Info info;
    u64 start_time;
    u64 microsecond_frequency;

    /* sync allocation */
    pthread_mutex_t object_mutex;
    Linux_Object    *free_objects;
} linux_state;

/* query cpu */
function u64 linux_get_cpu_frequency(void);


#ifndef NO_ENTRY_POINT

int main(int argc, char **argv) {

    linux_state.arena = arena_alloc(default_arena_capacity);

    /* setup the thread context */
    tctx = tctx_init(arena, str8_lit("Main"));

    /* allocate linux memory */
    linux_state.arena = arena_alloc(&arena, MiB(32));

    /* initialize runtime clock */
    linux_state.start_time = now_time_us();

    /* try to allow large pages */

    /* query system info */
    long processor_count = sysconf(_SC_NPROCESSORS_ONLN);

    System_Info *info = &linux_state.info;
    info->logical_processor_count = biggest(1, processor_count);
    info->page_size               = 0;
    info->large_page_size         = 0;
    info->allocation_granularity  = 0;
    info->cpu_frequency           = linux_get_cpu_frequency();

    /* sync object allocation */
    pthread_mutex_init(&linux_state.object_mutex, NULL);
    linux_state.object_arena = arena_alloc(&arena, MiB(8));

    /* initilize linux specific layers */
    // x11, wayland, ALSA, etc

    entry_point_caller(&arena, argc, argv);

    return 0;
}

#endif




/*== System API =========================================*/

/* wall clock time */
function u64 now_time_us(void) {
    u64 result = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    result = (u64)ts.tv_sec*1000000 + (u64)ts.tv_nsec/1000;
    return result;
}

function u64 time_elapsed_us(void) {
    return now_time_us() - linux_state.start_time;
}

function void sleep_ms(u32 ms) {
    struct timespec ts;
    ts.tv_sec = ms/1000;
    ts.tv_nsec = (ms % 1000)*1000000;
    nanosleep(&ts, NULL);
}

function System_Info *get_system_info(void) {
    return &linux_state.info;
}



/*== Threads ============================================*/

/* linux thread helpers */
function Linux_Object *linux_object_alloc(Linux_Object_Kind kind) {
    Linux_Object *result = NULL;
    pthread_mutex_lock(&linux_state.object_mutex);
    {
        result = linux_state.free_objects;
        if (result) {
            sll_stack_pop(linux_state.free_objects);
        } else {
            result = push_array_no_zero(&linux_state.object_arena, Linux_Object, 1);
        }
        debug_assert(result);
        mem_zero_struct(result);
    }
    pthread_mutex_unlock(&linux_state.object_mutex);
    result->kind = kind;
    return result;
}
function void linux_object_free(Linux_Object *obj) {
    obj->kind = Linux_Object_Kind_None;
    pthread_mutex_lock(&linux_state.object_mutex);
    sll_stack_push(linux_state.free_objects, obj);
    pthread_mutex_unlock(&linux_state.object_mutex);
}

/* linux thread entry point*/
function void *linux_thread_entry_point(void *ptr) {
    Linux_Object *obj = (Linux_Object *)ptr;
    Thread_Entry_Point_Func *thread_func = obj->thread.func;
    void *thread_params = obj->thread.params;
    thread_func(thread_params);
    return 0;
}

/* linux launch threads */
function Thread launch_thread(Thread_Entry_Point_Func *f, void *params) {
    Thread result = {0};
    Linux_Object *obj = linux_object_alloc(Linux_Object_Kind_Thread);
    obj->thread.func = f;
    obj->thread.params = params;
    int res = pthread_create(&obj->thread.handle, NULL, linux_thread_entry_point, obj);
    if (res == 0) {
        // success
        result.uint64[0] = int_from_ptr(obj);
    } else {
        linux_object_free(obj);
    }
    return result;
}
function void wait_join_thread(Thread thread) {
    Linux_Object *obj = (Linux_Object *)ptr_from_int(thread.uint64[0]);
    if (obj) {
        pthread_join(obj->thread.handle, NULL);
        linux_object_free(obj);
    }
}

/* linux barriers */
function Barrier barrier_alloc(u32 count) {
    Barrier result = {0};
    Linux_Object *obj = linux_object_alloc(Linux_Object_Kind_Barrier);
    if (obj) {
        pthread_barrier_init(&obj->barrier, NULL, count);
        result.uint64[0] = int_from_ptr(obj);
    }
    return result;
}
function void barrier_free(Barrier barrier) {
    Linux_Object *obj = (Linux_Object *)ptr_from_int(barrier.uint64[0]);
    if (obj) {
        pthread_barrier_destroy(&obj->barrier);
        linux_object_free(obj);
    }
}
function void barrier_wait(Barrier barrier) {
    Linux_Object *obj = (Linux_Object *)ptr_from_int(barrier.uint64[0]);
    if (obj) {
        pthread_barrier_wait(&obj->barrier);
    }
}

function u64 linux_get_cpu_frequency(void) {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) return 0.0;

    char line[256];
    double mhz = 0.0;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "cpu MHz", 7) == 0) {
            char *colon = strchr(line, ':');
            if (colon) {
                mhz = atof(colon + 1);
                break; // Stop at first core; nominal TSC frequency is global
            }
        }
    }
    fclose(fp);

    u64 result = (u64)(mhz*1000000);
    return result;
}

