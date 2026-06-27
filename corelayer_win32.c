#undef function
#include <windows.h>
#define function static
#pragma comment(lib, "advapi32")
#pragma comment(lib, "user32")

typedef enum {
    Win32_Object_Kind_None,
    Win32_Object_Kind_Thread,
    Win32_Object_Kind_Mutex,
    Win32_Object_Kind_RW_Mutex,
    Win32_Object_Kind_Condition,
    Win32_Object_Kind_Barrier,
} Win32_Object_Kind;

typedef struct Win32_Object Win32_Object;
struct Win32_Object {
    Win32_Object *next;
    Win32_Object_Kind kind;
    union {
        struct {
            Thread_Entry_Point_Func *func;
            void *params;
            HANDLE handle;
            DWORD tid;
        } thread;
        CRITICAL_SECTION mutex;
        SRWLOCK rw_mutex;
        CONDITION_VARIABLE cv;
        SYNCHRONIZATION_BARRIER barrier;
    };
};

/* pool allocation of sync objects */
function Win32_Object *win32_object_alloc(Win32_Object_Kind kind);
function void win32_object_free(Win32_Object *obj);

global struct {
    Arena *arena;

    /* info */
    System_Info info;
    u64 start_time;
    u64 microsecond_frequency;

    /* sync allocation */
    CRITICAL_SECTION object_mutex;
    Arena *object_arena;
    Win32_Object *free_objects;
} win32_state;


#ifndef NO_ENTRY_POINT

int main(int argc, char **argv) {

    /* initialize runtime clock */
    win32_state.microsecond_frequency = 1;
    LARGE_INTEGER freq;
    if (QueryPerformanceFrequency(&freq)) {
        win32_state.microsecond_frequency = freq.QuadPart;
    }
    win32_state.start_time = now_time_us();

    /* allocate windows memory */
    win32_state.arena = arena_alloc(MiB(32));

    /* setup the thread context */
    tctx = tctx_init(win32_state.arena, str8_lit("Main"));

    /* try to allow large pages */
    b32 large_pages_allowed = false;
    HANDLE token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        LUID luid;
        if (LookupPrivilegeValueA(0, SE_LOCK_MEMORY_NAME, &luid)) {
            TOKEN_PRIVILEGES priv;
            priv.PrivilegeCount           = 1;
            priv.Privileges[0].Luid       = luid;
            priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            large_pages_allowed = !!AdjustTokenPrivileges(token, 0, &priv, sizeof(priv), 0, 0);
        }
        CloseHandle(token);
    }

    /* query system info */
    SYSTEM_INFO sysinfo = {0};
    GetSystemInfo(&sysinfo);

    System_Info *info = &win32_state.info;
    info->logical_processor_count = (u64)sysinfo.dwNumberOfProcessors;
    info->page_size               = sysinfo.dwPageSize;
    info->large_page_size         = GetLargePageMinimum();
    info->allocation_granularity  = sysinfo.dwAllocationGranularity;

    /* sync object allocation */
    InitializeCriticalSection(&win32_state.object_mutex);
    win32_state.object_arena = arena_alloc(MiB(8));

    /* initialize windows specific layers */
    // window allocations
    // based on includes:
    //  - audio
    //  - gamepad

    entry_point_caller(win32_state.arena, argc, argv);

    return 0;
}

#endif // NO_ENTRY_POINT



/*== System API =========================================*/

/* wall clock time */
function u64 now_time_us(void) {
    u64 result = 0;
    LARGE_INTEGER counter;
    if (QueryPerformanceCounter(&counter)) {
        result = (counter.QuadPart*1000000)/win32_state.microsecond_frequency;
    }
    return result;
}

function u64 time_elapsed_us(void) {
    return now_time_us() - win32_state.start_time;
}

function void sleep_ms(u32 ms) {
    Sleep(ms);
}

function System_Info *get_system_info(void) {
    return &win32_state.info;
}



/*== Threads ============================================*/

/* win32 thread helpers */
function Win32_Object *win32_object_alloc(Win32_Object_Kind kind) {
    Win32_Object *result = NULL;
    EnterCriticalSection(&win32_state.object_mutex);
    {
        result = win32_state.free_objects;
        if (result) {
            sll_stack_pop(win32_state.free_objects);
        } else {
            result = push_array_no_zero(win32_state.object_arena, Win32_Object, 1);
        }
        debug_assert(result);
        mem_zero_struct(result);
    }
    LeaveCriticalSection(&win32_state.object_mutex);
    result->kind = kind;
    return result;
}
function void win32_object_free(Win32_Object *obj) {
    obj->kind = Win32_Object_Kind_None;
    EnterCriticalSection(&win32_state.object_mutex);
    sll_stack_push(win32_state.free_objects, obj);
    LeaveCriticalSection(&win32_state.object_mutex);
}

/* win32 thread entry point*/
function DWORD win32_thread_entry_point(void *ptr) {
    Win32_Object *obj = (Win32_Object *)ptr;
    Thread_Entry_Point_Func *thread_func = obj->thread.func;
    void *thread_ptr = obj->thread.params;
    thread_func(thread_ptr);
    return 0;
}

/* win32 launch threads */
function Thread launch_thread(Thread_Entry_Point_Func *f, void *params) {
    Thread result = {0};
    Win32_Object *obj = win32_object_alloc(Win32_Object_Kind_Thread);
    obj->thread.func = f;
    obj->thread.params = params;
    obj->thread.handle = CreateThread(0, 0, win32_thread_entry_point, obj, 0, &obj->thread.tid);
    result.uint64[0] = int_from_ptr(obj);
    return result;
}
function void wait_join_thread(Thread thread) {
    Win32_Object *obj = (Win32_Object *)ptr_from_int(thread.uint64[0]);
    if (obj) {
        WaitForSingleObject(obj->thread.handle, INFINITE);
        CloseHandle(obj->thread.handle);
        win32_object_free(obj);
    }
}

/* win32 mutex */
function Mutex mutex_alloc(void);
function void mutex_free(Mutex mutex);
function void mutex_lock(Mutex mutex);
function void mutex_unlock(Mutex mutex);

/* win32 rw_mutex */
function RW_Mutex rw_mutex_alloc(void);
function void rw_mutex_free(RW_Mutex mutex);
function void rw_mutex_lock(RW_Mutex mutex, b32 write_mode);
function void rw_mutex_unlock(RW_Mutex mutex, b32 write_mode);

/* win32 cond var */
function Condition cond_var_alloc(void);
function void cond_var_free(Condition cv);
function b32 cond_var_wait(Condition cv, Mutex mutex, u64 timeout_us);
function b32 cond_var_rw_wait(Condition cv, RW_Mutex mutex, b32 write_mode, u64 timeout_us);
function void cond_var_signal(Condition cv);
function void cond_var_broadcast(Condition cv);

/* win32 semaphores */
function Semaphore semaphore_alloc(u32 initial_count, u32 max_count);
function void semaphore_free(Semaphore sem);
function b32 semaphore_take(Semaphore sem, u64 timeout_us);
function void semaphore_release(Semaphore sem);

/* win32 barriers */
function Barrier barrier_alloc(u32 count) {
    Barrier result = {0};
    Win32_Object *obj = win32_object_alloc(Win32_Object_Kind_Barrier);
    if (obj) {
        InitializeSynchronizationBarrier(&obj->barrier, count, -1);
        result.uint64[0] = int_from_ptr(obj);
    }
    return result;
}
function void barrier_free(Barrier barrier) {
    Win32_Object *obj = (Win32_Object *)ptr_from_int(barrier.uint64[0]);
    if (obj) {
        DeleteSynchronizationBarrier(&obj->barrier);
        win32_object_free(obj);
    }
}
function void barrier_wait(Barrier barrier) {
    Win32_Object *obj = (Win32_Object *)ptr_from_int(barrier.uint64[0]);
    if (obj) {
        EnterSynchronizationBarrier(&obj->barrier, 0);
    }
}

