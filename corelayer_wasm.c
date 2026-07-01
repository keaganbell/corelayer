#include <emscripten/emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <pthread.h>



/*== WASM Code ==========================================*/

typedef enum { 
    WASM_Object_Kind_None,
    WASM_Object_Kind_Thread,
    WASM_Object_Kind_Mutex,
    WASM_Object_Kind_RW_Mutex,
    WASM_Object_Kind_Condition,
    WASM_Object_Kind_Barrier,
} WASM_Object_Kind;

typedef struct WASM_Object WASM_Object;
struct WASM_Object {
    WASM_Object *next;
    WASM_Object *prev;
    WASM_Object_Kind kind;
    union {
        struct {
            Thread_Entry_Point_Func *func;
            void *params;
            emscripten_wasm_worker_t handle;
            u32 worker_id;
        } thread;
        emscripten_lock_t   mutex;
        pthread_rwlock_t    rw_mutex;
        pthread_barrier_t   barrier;
    };
};

/* pool allocation of sync objects */
function WASM_Object *wasm_object_alloc(WASM_Object_Kind kind);
function void wasm_object_free(WASM_Object *obj);

global struct {
    Arena *arena;

    /* info */
    System_Info info;
    u64 start_time;
    u64 microsecond_frequency;

    /* sync allocation */
    emscripten_lock_t object_mutex;
    WASM_Object       *first_object;
    WASM_Object       *last_object;
    WASM_Object       *free_objects;
} wasm_state;

void main_loop(void);

int main(void) {
    wasm_state.arena = arena_alloc();
    wasm_state.object_mutex = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;
    entry_point_caller(wasm_state.arena, 0, 0);
    //emscripten_set_main_loop(main_loop, 0, true);
    return 0;
}


/*== System API =========================================*/

/* wall clock time */
function u64 now_time_us(void) {
    double time = emscripten_get_now();
    u64 result = (u64)(time*1000);
    return result;
}



/*== Threads =========================================== */

/* wasm thread helpers */
function WASM_Object *wasm_object_alloc(WASM_Object_Kind kind) {
    WASM_Object *result = NULL;
    emscripten_lock_wait_acquire(&wasm_state.object_mutex, max_u64);
    {
        result = wasm_state.free_objects;
        if (result) {
            sll_stack_pop(wasm_state.free_objects);
        } else {
            result = push_array_no_zero(wasm_state.arena, WASM_Object, 1);
            dll_push_back(wasm_state.first_object, wasm_state.last_object, result);
        }
        debug_assert(result);
        mem_zero_struct(result);
    }
    emscripten_lock_release(&wasm_state.object_mutex);
    result->kind = kind;
    return result;
}
function void wasm_object_free(WASM_Object *obj) {
    obj->kind = WASM_Object_Kind_None;
    emscripten_lock_wait_acquire(&wasm_state.object_mutex, max_u64);
    dll_remove(wasm_state.first_object, wasm_state.last_object, obj);
    sll_stack_push(wasm_state.free_objects, obj);
    emscripten_lock_release(&wasm_state.object_mutex);
}

/* wasm thread entry point*/
function void wasm_thread_entry_point(void) {
    WASM_Object *obj = NULL;
    for (WASM_Object *n = wasm_state.first_object; n != NULL; n = n->next ) {
        if (n->kind == WASM_Object_Kind_Thread) {
            if (n->thread.handle == emscripten_wasm_worker_self_id()) {
                obj = n;
                break;
            }
        }
    }
    if (obj) {
        Thread_Entry_Point_Func *thread_func = obj->thread.func;
        void *thread_params = obj->thread.params;
        thread_func(thread_params);
    } else {
        printf("(WASM) thread_entry_point - ERROR: failed to find thread object for worker %d\n", emscripten_wasm_worker_self_id());
    }
}

/* wasm launch threads */
function Thread launch_thread(Thread_Entry_Point_Func *f, void *params) {
    Thread result = {0};
    WASM_Object *obj = wasm_object_alloc(WASM_Object_Kind_Thread);
    if (obj) {
        obj->thread.func = f;
        obj->thread.params = params;
        // NOTE: emscripten_malloc_wasm_worker LEAKS MEMORY!?
        // there is no emscripten_free_wasm_worker...
        // TODO: create my own free list of wasm_workers_t
        obj->thread.handle = emscripten_malloc_wasm_worker(MiB(8)); // stack size
        emscripten_wasm_worker_post_function_v(obj->thread.handle, wasm_thread_entry_point);
    }
    return result;
}
function void wait_join_thread(Thread thread) {
    WASM_Object *obj = (WASM_Object *)ptr_from_int(thread.uint64[0]);
    if (obj) {
        wasm_object_free(obj);
    }
}
#if 0
function b32 try_join_thread(Thread thread) {
    b32 result = false;
    WASM_Object *obj = (WASM_Object *)ptr_from_int(thread.uint64[0]);
    if (obj) {
        if (0 == pthread_tryjoin_np(obj->thread.handle, NULL)) {
            result = true;
            wasm_object_free(obj);
        }
    }
    return result;
}
#endif

/* wasm barriers */
function Barrier barrier_alloc(u32 count) {
    Barrier result = {0};
    WASM_Object *obj = wasm_object_alloc(WASM_Object_Kind_Barrier);
    if (obj) {
        pthread_barrier_init(&obj->barrier, NULL, count);
        result.uint64[0] = int_from_ptr(obj);
    }
    return result;
}
function void barrier_free(Barrier barrier) {
    WASM_Object *obj = (WASM_Object *)ptr_from_int(barrier.uint64[0]);
    if (obj) {
        pthread_barrier_destroy(&obj->barrier);
        wasm_object_free(obj);
    }
}
function void barrier_wait(Barrier barrier) {
    WASM_Object *obj = (WASM_Object *)ptr_from_int(barrier.uint64[0]);
    if (obj) {
        pthread_barrier_wait(&obj->barrier);
    }
}

/* memory allocations */
function void *aligned_malloc(u64 size, u64 align) {
    void *result = NULL;
    if (size > 0 ) {
        if (align >= sizeof(void*) && (align & (align - 1)) == 0) {
            void *p;
            int res = posix_memalign(&p, (size_t)align, (size_t)size);
            if (res == 0) {
                result = p;
            }
        }
    }
    return result;
}
function void aligned_free(void *ptr) {
    free(ptr);
}

