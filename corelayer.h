#ifndef CORELAYER_H
#define CORELAYER_H


/*== Foreign Includes ================================*/

#define _CRT_SECURE_nO_WARNINGS

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


/*== COMPILER, OS, ARCH CONTEXT =========================*/

/* Clang */
#if defined(__clang__)

# define COMPILER_CLANG 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__gnu_linux__) || defined(__linux__)
#  define OS_LINUX 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# else
#  error This compiler/OS combo is not supported.
# endif

# if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#  define ARCH_X64 1
# elif defined(i386) || defined(__i386) || defined(__i386__)
#  define ARCH_X86 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# elif defined(__arm__)
#  define ARCH_ARM32 1
# else
#  error Architecture not supported.
# endif

/* MSVC */
#elif defined(_MSC_VER)

# define COMPILER_MSVC 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# else
#  error This compiler/OS combo is not supported.
# endif

# if defined(_M_AMD64)
#  define ARCH_X64 1
# elif defined(_M_IX86)
#  define ARCH_X86 1
# elif defined(_M_ARM64)
#  define ARCH_ARM64 1
# elif defined(_M_ARM)
#  define ARCH_ARM32 1
# else
#  error Architecture not supported.
# endif

/* GCC */
#elif defined(__GNUC__) || defined(__GNUG__)

# define COMPILER_GCC 1

# if defined(__gnu_linux__) || defined(__linux__)
#  define OS_LINUX 1
# else
#  error This compiler/OS combo is not supported.
# endif

# if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#  define ARCH_X64 1
# elif defined(i386) || defined(__i386) || defined(__i386__)
#  define ARCH_X86 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# elif defined(__arm__)
#  define ARCH_ARM32 1
# else
#  error Architecture not supported.
# endif

#else
# error Compiler not supported.
#endif


/* Architecture */
#if defined(ARCH_X64)
# define ARCH_64BIT 1
#elif defined(ARCH_X86)
# define ARCH_32BIT 1
#endif

#if ARCH_ARM32 || ARCH_ARM64 || ARCH_X64 || ARCH_X86
# define ARCH_LITTLE_ENDIAN 1
#else
# error Endianness of this architecture not understood.
#endif



/*== Compiler Warnings ==================================*/

#if COMPILER_MSVC
/* negative sign on unsigned type */
#pragma warning(disable: 4146)

/* trancation double to float */
#pragma warning(disable: 4305)
#endif



/*== Aliases ============================================*/

/* C array count */
#if !defined(arrcount)
# define arrcount(a) (sizeof(a)/sizeof(a[0]))
#endif

/* offsetof */
#if !defined(offsetof)
# define offsetof(t, m) (&(((t *)NULL)->m))
#endif

/* alignof */
#if !defined(alignof)
# if defined(COMPILER_MSVC)
#  define alignof(T) __alignof(T)
# elif defined(COMPILER_CLANG)
#  define alignof(T) __alignof(T)
# elif defined(COMPILER_GCC)
#  define alignof(T) __alignof__(T)
# else
#  error alignof not defined for this compiler.
# endif
#endif

/* align struct type */
#if COMPILER_MSVC
# define aligntype(x) __declspec(align(x))
#elif COMPILER_CLANG || COMPILER_GCC
# define aligntype(x) __attribute__((aligned(x)))
#else
# error aligntype not defined for this compiler.
#endif

/* thread static */
#if !defined(thread_static)
# if defined(COMPILER_MSVC)
#  define thread_static __declspec(thread)
# elif defined(COMPILER_CLANG)
#  define thread_static __thread
# elif defined(COMPILER_GCC)
#  define thread_static __thread
# else
#  error thread_static not defined for this compiler.
# endif
#endif

/* global static */
#define global static

/* read only */
#if COMPILER_MSVC || (COMPILER_CLANG && OS_WINDOWS)
# pragma section(".rdata$", read)
# define read_only __declspec(allocate(".rdata$"))
#elif (COMPILER_CLANG && OS_LINUX)
# define read_only __attribute__((section(".rodata")))
#else
# define read_only
#endif


/* true / false */
#ifndef true
#define true (1!=0)
#endif
#ifndef false
#define false (1==0)
#endif




/*== Metaprogramming ====================================*/

#define function static
#define exposed_function




/*== Types ==============================================*/

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef uint8_t     b8;
typedef uint16_t    b16;
typedef uint32_t    b32;
typedef uint64_t    b64;

typedef float   f32;
typedef double  f64;




/*== Math Types =========================================*/

/* conversion */
#define KiB(x) (1024ull*(x))
#define MiB(x) (1024ull*KiB(x))
#define GiB(x) (1024ull*MiB(x))
#define deg_to_rads(x) ((x)*pi32/180.0f)
#define rads_to_deg(x) ((x)*180.0f/pi32)

/* min, max, clamp */
#define smallest(a,b) ((a)<(b)?(a):(b))
#define biggest(a,b) ((a)>(b)?(a):(b))
#define clamp_top(v,b) smallest(v,b)
#define clamp_bot(v,a) biggest(v,a)
#define constrain(v, a, b) smallest(biggest((v), (a)), (b))

/* function wrappers */
#define pow_f32(b, e) powf((b), (e))

/* lerp */
function f32 f32_lerp(f32 a, f32 b, f32 t);
function u32 u32_lerp(u32 a, u32 b, f32 t);


/* range types */
typedef union {
    struct {
        u8 min;
        u8 max;
    };
    u8 v[2];
} Range_U8;

typedef union {
    struct {
        u16 min;
        u16 max;
    };
    u16 v[2];
} Range_U16;

typedef union {
    struct {
        u32 min;
        u32 max;
    };
    u32 v[2];
} Range_U32;

typedef union {
    struct {
        u64 min;
        u64 max;
    };
    u64 v[2];
} Range_U64;

/* evenyl distributes m_count into n_count ranges and returns the n_idx'th range */
function Range_U64 m_range_from_n_index_m_count(u64 n_idx, u64 n_count, u64 m_count);

/* range helpers */
#define range_size(r) ((r).max - (r).min)
#define range_contains(r, x) ((r).min <= (x) && (x) <= (r).max)
#define range_surrounds(r, x) ((r).min < (x) && (x) < (r).max)


/* Vectors */
typedef union {
    struct { f32   x;   f32 y; };
    struct { f32 min; f32 max; };
    f32 v[2];
} Vec2f32, Range_f32;

typedef union {
    struct { f32 x; f32 y; f32 z; };
    struct { f32 r; f32 g; f32 b; };
    f32 v[3];
} Vec3f32;

typedef union {
    struct { f32 x; f32 y; f32 z; f32 w; };
    struct { f32 r; f32 g; f32 b; f32 a; };
    struct {  Vec2f32 min;  Vec2f32 max; };
    f32 v[4];
} Vec4f32, Range_Vec2f32;

/* vector constructors */
function Vec2f32 vec2(f32 x, f32 y);
function Vec3f32 vec3(f32 x, f32 y, f32 z);
function Vec4f32 vec4(f32 x, f32 y, f32 z, f32 w);

/* range constructors */
function Range_f32 range_f32(f32 min, f32 max);

/* vec3 operations */
function Vec3f32 vec3_add(Vec3f32 a, Vec3f32 b);
function Vec3f32 vec3_sub(Vec3f32 a, Vec3f32 b);
function Vec3f32 vec3_mix(Vec3f32 a, Vec3f32 b);
function Vec3f32 vec3_scale(Vec3f32 v, f32 s);
function Vec3f32 vec3_negate(Vec3f32 v);
function Vec3f32 vec3_absf(Vec3f32 v);
function Vec3f32 vec3_lerp(Vec3f32 a, Vec3f32 b, f32 t);
function Vec3f32 vec3_cross(Vec3f32 a, Vec3f32 b);
function f32 vec3_dot(Vec3f32 a, Vec3f32 b);
function f32 vec3_length(Vec3f32 v);
function Vec3f32 vec3_norm(Vec3f32 v);
function b32 vec3_near_zero(Vec3f32 v);
function Vec3f32 vec3_reflect(Vec3f32 v, Vec3f32 n);

/* color operations */
function Vec3f32 linear_from_srgb(Vec3f32 srgb);
function Vec3f32 srgb_from_linear(Vec3f32 linear);


/* quaternions */
typedef struct { 
    f32 a; f32 b; f32 c; f32 d; 
} Quaternion;
function Quaternion quat(Vec3f32 axis, f32 deg);
function Quaternion quat_conjugate(Quaternion q);
function Quaternion quat_inverse(Quaternion q, b32 *exists);
function Quaternion quat_mul(Quaternion q, Quaternion p);
function Vec3f32 rotate(Quaternion quat, Vec3f32 p);



/*== Pseudo RNG =========================================*/

typedef struct {
    u64 state;
    u64 inc;
} Rand_State;

/* NOTE: Thread_Context has a Rand_State */

/* seeding functions */
function void rand_seed(u64 initstate, u64 initseq);
function void rand_seed_r(Rand_State* rng, u64 initstate, u64 initseq);

/* generate random ints */
function u32 rand_u32(void);
function u32 rand_u32_r(Rand_State *rng);
function i32 rand_i32_in_range(i32 min, i32 max);
function i32 rand_i32_in_range_r(Rand_State *rng, i32 min, i32 max);

/* generated random float [0.0, 1.0] */
function f32 rand_f32(void);
function f32 rand_f32_r(Rand_State *rng);

/* random vectors */
function Vec3f32 rand_unit_vec3(void);
function Vec3f32 rand_unit_vec3_r(Rand_State *rng);
function Vec3f32 rand_vec3_in_unit_disk(void);
function Vec3f32 rand_vec3_in_unit_disk_r(Rand_State *rng);

/* random color */
function Vec3f32 rand_color(void);



/*== Loop Macros ========================================*/

#define defer_loop(begin, end) for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))




/*== Memory Operations ==================================*/

/* memory ops */
#define mem_copy(dst, src, size) memmove((dst), (src), (size))
#define mem_set(dst, byte, size) memset((dst), (byte), (size))
#define mem_compare(a, b, size)  memcmp((a), (b), (size))

/* memory copy */
#define mem_copy_struct(d, s)    mem_copy((d), (s), sizeof(*(d)))
#define mem_copy_array(d, a)     mem_copy((d), (s), sizeof(d))
#define mem_copy_typed(d, a, c)  mem_copy((d), (s), (c)*sizeof(*(d)))
#define mem_copy_str8(dst, s)    mem_copy((dst), (s).ptr, (s).length)

/* memory zero */
#define mem_zero(s, z)       mem_set((s), 0, (z))
#define mem_zero_struct(s)   mem_zero((s), sizeof(*(s)))
#define mem_zero_array(a)    mem_zero((a), sizeof(a))
#define mem_zero_typed(m, c) mem_zero((m), (c)*sizeof(*(m)))

/* memory match */
#define mem_match(a, b, z)      (mem_compare((a), (b), (z)) == 0)
#define mem_match_struct(a, b)  mem_match((a), (b), sizeof(*(a)))
#define mem_match_array(a, b)   mem_match((a), (b), sizeof(a))

/* alignment */
#define align_pow2(x,b)         (((x) + (b) - 1)&(~((b) - 1)))
#define align_down_pow2(x,b)    ((x)&(~((b) - 1)))
#define align_pad_pow2(x,b)     ((0-(x)) & ((b) - 1))
#define is_pow2(x)              ((x)!=0 && ((x)&((x)-1))==0)
#define is_pow2_or_zero(x)      ((((x) - 1)&(x)) == 0)



/*== Memory Allocation ==================================*/

function void *virtual_alloc(u64 size);
function void *virtual_reserve(u64 size);
function void virtual_commit(void *ptr, u64 size);

function void aligned_malloc(u64 size, u64 align);
function void aligned_realloc(u64 size, u64 align);
function void aligned_free(void *ptr);



/*== Asserts ============================================*/

/* debug trap */
#if COMPILER_MSVC
# define debug_trap() __debugbreak()
#elif COMPILER_CLANG || COMPILER_GCC
# define debug_trap() __builtin_trap()
#endif

/* always assert */
#define assert_always(x) do{if(!(x)) {debug_trap();}}while(0)

/* debug assert */
#if BUILD_DEBUG
# define debug_assert(x) assert_always(x)
#else
# define debug_assert(x) (void)(x)
#endif

/* invalid codepath */
#define invalid_codepath() debug_assert(!"Invalid Codepath!")

/* not yet implemented */
#define not_yet_implemented() debug_assert(!"Not Yet Implemented!")

/* no-op */
#define no_op ((void)0)

/* static assert */
#define static_assert(c, id) global u8 macro_concat(id, __LINE__)[(c)?1:-1]



/*== Misc Helper Macros =================================*/

#define stringify_(s) #s
#define stringify(s) stringify_(s)

#define macro_concat_(a,b) a##b
#define macro_concat(a,b) macro_concat_(a,b)

#if ARCH_64BIT
# define int_from_ptr(ptr) ((u64)(ptr))
#elif ARCH_32BIT
# define int_from_ptr(ptr) ((u32)(ptr))
#else
# error Missing pointer-to-integer cast for this architecture.
#endif
#define ptr_from_int(i) (void*)(i)



/*== Atomic Operations ==================================*/

#if COMPILER_MSVC
# include <intrin.h>
# if ARCH_X64
#  define ins_atomic_u128_eval_cond_assign(x,k,c) (b32)_InterlockedCompareExchange128((__int64 *)(x), ((__int64 *)&(k))[1], ((__int64 *)&(k))[0], (__int64 *)(c))
#  define ins_atomic_u64_eval(x)                  (u64)__iso_volatile_load64((__int64*)(x))
#  define ins_atomic_u64_inc_eval(x)              _InterlockedIncrement64((__int64 *)(x))
#  define ins_atomic_u64_dec_eval(x)              _InterlockedDecrement64((__int64 *)(x))
#  define ins_atomic_u64_eval_assign(x,c)         _InterlockedExchange64((__int64 *)(x), (c))
#  define ins_atomic_u64_add_eval(x,c)            _interlockedadd64((__int64 *)(x), (c))
#  define ins_atomic_u64_eval_cond_assign(x,k,c)  _InterlockedCompareExchange64((__int64 *)(x), (k), (c))
#  define ins_atomic_u32_eval(x)                  (u32)__iso_volatile_load32((__int32*)(x))
#  define ins_atomic_u32_inc_eval(x)              _InterlockedIncrement((long *)(x))
#  define ins_atomic_u32_dec_eval(x)              _InterlockedDecrement((long *)(x))
#  define ins_atomic_u32_eval_assign(x,c)         _InterlockedExchange((long *)(x), (c))
#  define ins_atomic_u32_eval_cond_assign(x,k,c)  _InterlockedCompareExchange((long *)(x), (k), (c))
#  define ins_atomic_u32_add_eval(x,c)            _interlockedadd((long *)(x), (c))
#  define ins_atomic_u8_eval_assign(x,c)          _InterlockedExchange8((char *)(x), (char)(c))
#  define ins_atomic_u8_or(x,c)                   _InterlockedOr8((char *)(x), (char)(c))
#  define ins_atomic_u32_or(x,c)                  _InterlockedOr((long *)(x), (long)(c))
# else
#  error Atomic intrinsics not defined for this compiler / architecture combination.
# endif
#elif COMPILER_CLANG || COMPILER_GCC
#  define ins_atomic_u128_eval_cond_assign(x,k,c) (b32)__atomic_compare_exchange_n((__int128 *)(x),(__int128 *)(c),*(__int128 *)(k),0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)
#  define ins_atomic_u64_eval(x)                  __atomic_load_n((u64 *)(x), __ATOMIC_SEQ_CST)
#  define ins_atomic_u64_inc_eval(x)              __atomic_add_fetch((u64 *)(x), 1, __ATOMIC_SEQ_CST)
#  define ins_atomic_u64_dec_eval(x)              __atomic_sub_fetch((u64 *)(x), 1, __ATOMIC_SEQ_CST)
#  define ins_atomic_u64_eval_assign(x,c)         __atomic_exchange_n(x, c, __ATOMIC_SEQ_CST)
#  define ins_atomic_u64_add_eval(x,c)            __atomic_add_fetch((u64 *)(x), c, __ATOMIC_SEQ_CST)
#  define ins_atomic_u64_eval_cond_assign(x,k,c)  ({ u64 _new = (c); __atomic_compare_exchange_n((U64 *)(x),&_new,(k),0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST); _new; })
#  define ins_atomic_u32_eval(x)                  __atomic_load_n(x, __ATOMIC_SEQ_CST)
#  define ins_atomic_u32_inc_eval(x)              __atomic_add_fetch((u32 *)(x), 1, __ATOMIC_SEQ_CST)
#  define ins_atomic_u32_dec_eval(x)              __atomic_sub_fetch((u32 *)(x), 1, __ATOMIC_SEQ_CST)
#  define ins_atomic_u32_add_eval(x,c)            __atomic_add_fetch((u32 *)(x), c, __ATOMIC_SEQ_CST)
#  define ins_atomic_u32_eval_assign(x,c)         __atomic_exchange_n((x), (c), __ATOMIC_SEQ_CST)
#  define ins_atomic_u32_eval_cond_assign(x,k,c)  ({ u32 _new = (c); __atomic_compare_exchange_n((U32 *)(x),&_new,(k),0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST); _new; })
#  define ins_atomic_u8_eval_assign(x,c)          __atomic_exchange_n((x), (c), __ATOMIC_SEQ_CST)
#  define ins_atomic_u8_or(x,c)                   __atomic_fetch_or((u8 *)(x), (u8)(c), __ATOMIC_SEQ_CST)
#  define ins_atomic_u32_or(x,c)                  __atomic_fetch_or((u32 *)(x), (u32)(c), __ATOMIC_SEQ_CST)
#else
#  error Atomic intrinsics not defined for this compiler / architecture.
#endif

#if ARCH_64BIT
# define ins_atomic_ptr_eval_cond_assign(x,k,c) (void *)ins_atomic_u64_eval_cond_assign((u64 *)(x), (u64)(k), (u64)(c))
# define ins_atomic_ptr_eval_assign(x,c)        (void *)ins_atomic_u64_eval_assign((u64 *)(x), (u64)(c))
# define ins_atomic_ptr_eval(x)                 (void *)ins_atomic_u64_eval((u64 *)x)
#else
# error Atomic intrinsics for pointers not defined for this architecture.
#endif


/*== CPU Counters =======================================*/

#if ARCH_X64 || ARCH_X64
# define read_cpu_timestamp() __rdtsc()
#else
# error CPU timer NYI for this architecture
#endif


/*== Nil Check Macros ===================================*/

#define check_nil(nil,p) ((p) == 0 || (p) == nil)
#define set_nil(nil,p) ((p) = nil)


/*== Doubly-Linked List =================================*/

/* dll insert */
#define dll_insert_npz(nil,f,l,p,n,next,prev) (check_nil(nil,f) ? \
((f) = (l) = (n), set_nil(nil,(n)->next), set_nil(nil,(n)->prev)) :\
check_nil(nil,p) ? \
((n)->next = (f), (f)->prev = (n), (f) = (n), set_nil(nil,(n)->prev)) :\
((p)==(l)) ? \
((l)->next = (n), (n)->prev = (l), (l) = (n), set_nil(nil, (n)->next)) :\
(((!check_nil(nil,p) && check_nil(nil,(p)->next)) ? (0) : ((p)->next->prev = (n))), ((n)->next = (p)->next), ((p)->next = (n)), ((n)->prev = (p))))

/* dll push back */
#define dll_push_back_npz(nil,f,l,n,next,prev) dll_insert_npz(nil,f,l,l,n,next,prev)

/* dll push front */
#define dll_push_front_npz(nil,f,l,n,next,prev) dll_insert_npz(nil,l,f,f,n,prev,next)

/* dll push remove */
#define dll_remove_npz(nil,f,l,n,next,prev) (((n) == (f) ? (f) = (n)->next : (0)),\
((n) == (l) ? (l) = (l)->prev : (0)),\
(check_nil(nil,(n)->prev) ? (0) :\
((n)->prev->next = (n)->next)),\
(check_nil(nil,(n)->next) ? (0) :\
((n)->next->prev = (n)->prev)))

/* dll helpers */
#define dll_insert_np(f,l,p,n,next,prev) dll_insert_npz(0,f,l,p,n,next,prev)
#define dll_push_back_np(f,l,n,next,prev) dll_push_back_npz(0,f,l,n,next,prev)
#define dll_push_front_np(f,l,n,next,prev) dll_push_front_npz(0,f,l,n,next,prev)
#define dll_remove_np(f,l,n,next,prev) dll_remove_npz(0,f,l,n,next,prev)
#define dll_insert(f,l,p,n) dll_insert_npz(0,f,l,p,n,next,prev)
#define dll_push_back(f,l,n) dll_push_back_npz(0,f,l,n,next,prev)
#define dll_push_front(f,l,n) dll_push_front_npz(0,f,l,n,next,prev)
#define dll_remove(f,l,n) dll_remove_npz(0,f,l,n,next,prev)



/*== Singly-Linked List =================================*/

/* sll queue push */
#define sll_queue_push_nz(nil,f,l,n,next) (check_nil(nil,f)?\
((f)=(l)=(n),set_nil(nil,(n)->next)):\
((l)->next=(n),(l)=(n),set_nil(nil,(n)->next)))

/* sll queue push front */
#define sll_queue_push_front_nz(nil,f,l,n,next) (check_nil(nil,f)?\
((f)=(l)=(n),set_nil(nil,(n)->next)):\
((n)->next=(f),(f)=(n)))

/* sll queue pop */
#define sll_queue_pop_nz(nil,f,l,next) ((f)==(l)?\
(set_nil(nil,f),set_nil(nil,l)):\
((f)=(f)->next))

/* sll queue helpers */
#define sll_queue_push_n(f,l,n,next) sll_queue_push_nz(0,f,l,n,next)
#define sll_queue_push_front_n(f,l,n,next) sll_queue_push_front_nz(0,f,l,n,next)
#define sll_queue_pop_n(f,l,next) sll_queue_pop_nz(0,f,l,next)
#define sll_queue_push(f,l,n) sll_queue_push_nz(0,f,l,n,next)
#define sll_queue_push_front(f,l,n) sll_queue_push_front_nz(0,f,l,n,next)
#define sll_queue_pop(f,l) sll_queue_pop_nz(0,f,l,next)


/* sll stack push */
#define sll_stack_push_n(f,n,next) ((n)->next=(f), (f)=(n))

/* sll stack pop */
#define sll_stack_pop_n(f,next) ((f)=(f)->next)

/* sll stack helpers */
#define sll_stack_push(f,n) sll_stack_push_n(f,n,next)
#define sll_stack_pop(f) sll_stack_pop_n(f,next)



/*== Basic Constants ====================================*/

global const u32 sign32     = 0x80000000;
global const u32 exponent32 = 0x7F800000;
global const u32 mantissa32 = 0x007FFFFF;

global const f32   big_golden32 = 1.61803398875f;
global const f32 small_golden32 = 0.61803398875f;

global const f32 pi32 = 3.1415926535897f;

global const f64 machine_epsilon64 = 4.94065645841247e-324;

global const u64 max_u64 = 0xffffffffffffffffull;
global const u32 max_u32 = 0xffffffff;
global const u16 max_u16 = 0xffff;
global const u8  max_u8  = 0xff;

global const i64 max_i64 = (i64)0x7fffffffffffffffll;
global const i32 max_i32 = (i32)0x7fffffff;
global const i16 max_i16 = (i16)0x7fff;
global const i8  max_i8  =  (i8)0x7f;

global const i64 min_i64 = (i64)0x8000000000000000ll;
global const i32 min_i32 = (i32)0x80000000;
global const i16 min_i16 = (i16)0x8000;
global const i8  min_i8  =  (i8)0x80;

global const f32 max_f32 = (f32)0x7f7fffff;
global const f64 max_f64 = (f64)0x7fefffffffffffff;

global const u32 bitmask1  = 0x00000001;
global const u32 bitmask2  = 0x00000003;
global const u32 bitmask3  = 0x00000007;
global const u32 bitmask4  = 0x0000000f;
global const u32 bitmask5  = 0x0000001f;
global const u32 bitmask6  = 0x0000003f;
global const u32 bitmask7  = 0x0000007f;
global const u32 bitmask8  = 0x000000ff;
global const u32 bitmask9  = 0x000001ff;
global const u32 bitmask10 = 0x000003ff;
global const u32 bitmask11 = 0x000007ff;
global const u32 bitmask12 = 0x00000fff;
global const u32 bitmask13 = 0x00001fff;
global const u32 bitmask14 = 0x00003fff;
global const u32 bitmask15 = 0x00007fff;
global const u32 bitmask16 = 0x0000ffff;
global const u32 bitmask17 = 0x0001ffff;
global const u32 bitmask18 = 0x0003ffff;
global const u32 bitmask19 = 0x0007ffff;
global const u32 bitmask20 = 0x000fffff;
global const u32 bitmask21 = 0x001fffff;
global const u32 bitmask22 = 0x003fffff;
global const u32 bitmask23 = 0x007fffff;
global const u32 bitmask24 = 0x00ffffff;
global const u32 bitmask25 = 0x01ffffff;
global const u32 bitmask26 = 0x03ffffff;
global const u32 bitmask27 = 0x07ffffff;
global const u32 bitmask28 = 0x0fffffff;
global const u32 bitmask29 = 0x1fffffff;
global const u32 bitmask30 = 0x3fffffff;
global const u32 bitmask31 = 0x7fffffff;
global const u32 bitmask32 = 0xffffffff;

global const u64 bitmask33 = 0x00000001ffffffffull;
global const u64 bitmask34 = 0x00000003ffffffffull;
global const u64 bitmask35 = 0x00000007ffffffffull;
global const u64 bitmask36 = 0x0000000fffffffffull;
global const u64 bitmask37 = 0x0000001fffffffffull;
global const u64 bitmask38 = 0x0000003fffffffffull;
global const u64 bitmask39 = 0x0000007fffffffffull;
global const u64 bitmask40 = 0x000000ffffffffffull;
global const u64 bitmask41 = 0x000001ffffffffffull;
global const u64 bitmask42 = 0x000003ffffffffffull;
global const u64 bitmask43 = 0x000007ffffffffffull;
global const u64 bitmask44 = 0x00000fffffffffffull;
global const u64 bitmask45 = 0x00001fffffffffffull;
global const u64 bitmask46 = 0x00003fffffffffffull;
global const u64 bitmask47 = 0x00007fffffffffffull;
global const u64 bitmask48 = 0x0000ffffffffffffull;
global const u64 bitmask49 = 0x0001ffffffffffffull;
global const u64 bitmask50 = 0x0003ffffffffffffull;
global const u64 bitmask51 = 0x0007ffffffffffffull;
global const u64 bitmask52 = 0x000fffffffffffffull;
global const u64 bitmask53 = 0x001fffffffffffffull;
global const u64 bitmask54 = 0x003fffffffffffffull;
global const u64 bitmask55 = 0x007fffffffffffffull;
global const u64 bitmask56 = 0x00ffffffffffffffull;
global const u64 bitmask57 = 0x01ffffffffffffffull;
global const u64 bitmask58 = 0x03ffffffffffffffull;
global const u64 bitmask59 = 0x07ffffffffffffffull;
global const u64 bitmask60 = 0x0fffffffffffffffull;
global const u64 bitmask61 = 0x1fffffffffffffffull;
global const u64 bitmask62 = 0x3fffffffffffffffull;
global const u64 bitmask63 = 0x7fffffffffffffffull;
global const u64 bitmask64 = 0xffffffffffffffffull;

global const u32 bit1  = (1<<0);
global const u32 bit2  = (1<<1);
global const u32 bit3  = (1<<2);
global const u32 bit4  = (1<<3);
global const u32 bit5  = (1<<4);
global const u32 bit6  = (1<<5);
global const u32 bit7  = (1<<6);
global const u32 bit8  = (1<<7);
global const u32 bit9  = (1<<8);
global const u32 bit10 = (1<<9);
global const u32 bit11 = (1<<10);
global const u32 bit12 = (1<<11);
global const u32 bit13 = (1<<12);
global const u32 bit14 = (1<<13);
global const u32 bit15 = (1<<14);
global const u32 bit16 = (1<<15);
global const u32 bit17 = (1<<16);
global const u32 bit18 = (1<<17);
global const u32 bit19 = (1<<18);
global const u32 bit20 = (1<<19);
global const u32 bit21 = (1<<20);
global const u32 bit22 = (1<<21);
global const u32 bit23 = (1<<22);
global const u32 bit24 = (1<<23);
global const u32 bit25 = (1<<24);
global const u32 bit26 = (1<<25);
global const u32 bit27 = (1<<26);
global const u32 bit28 = (1<<27);
global const u32 bit29 = (1<<28);
global const u32 bit30 = (1<<29);
global const u32 bit31 = (1<<30);
global const u32 bit32 = (1<<31);

global const u64 bit33 = (1ull<<32);
global const u64 bit34 = (1ull<<33);
global const u64 bit35 = (1ull<<34);
global const u64 bit36 = (1ull<<35);
global const u64 bit37 = (1ull<<36);
global const u64 bit38 = (1ull<<37);
global const u64 bit39 = (1ull<<38);
global const u64 bit40 = (1ull<<39);
global const u64 bit41 = (1ull<<40);
global const u64 bit42 = (1ull<<41);
global const u64 bit43 = (1ull<<42);
global const u64 bit44 = (1ull<<43);
global const u64 bit45 = (1ull<<44);
global const u64 bit46 = (1ull<<45);
global const u64 bit47 = (1ull<<46);
global const u64 bit48 = (1ull<<47);
global const u64 bit49 = (1ull<<48);
global const u64 bit50 = (1ull<<49);
global const u64 bit51 = (1ull<<50);
global const u64 bit52 = (1ull<<51);
global const u64 bit53 = (1ull<<52);
global const u64 bit54 = (1ull<<53);
global const u64 bit55 = (1ull<<54);
global const u64 bit56 = (1ull<<55);
global const u64 bit57 = (1ull<<56);
global const u64 bit58 = (1ull<<57);
global const u64 bit59 = (1ull<<58);
global const u64 bit60 = (1ull<<59);
global const u64 bit61 = (1ull<<60);
global const u64 bit62 = (1ull<<61);
global const u64 bit63 = (1ull<<62);
global const u64 bit64 = (1ull<<63);


/* for string <-> integer conversion */
read_only global u8 integer_symbols[16] = {
  '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',
};

/* includes reverses for uppercase and lowercase hex. */
read_only global u8 integer_symbol_reverse[128] = {
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

read_only global u8 base64[64] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '_', '$',
};

read_only global u8 base64_reverse[128] =
{
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0x3F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
  0xFF,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,
  0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0xFF,0xFF,0xFF,0xFF,0x3E,
  0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
  0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0xFF,0xFF,0xFF,0xFF,0xFF,
};


/*== Arenas =============================================*/

global const u64 default_arena_capacity = MiB(4);
global const u64 default_arena_page_alignment = KiB(4);

typedef struct Arena Arena;
struct Arena {
    Arena *prev;
    Arena *current;

    // global offset
    u64 offset;

    // local offset
    void *base;
    u64 pos;
    u64 cap;
};

#define ARENA_HEADER_SIZE 128
static_assert(sizeof(Arena) <= ARENA_HEADER_SIZE, arena_header_size_check);

/* arena api */
function Arena *arena_alloc(u64 min_cap);
function void arena_free(Arena *arena);
function void arena_reset(Arena *arena);
function void arena_rewind(Arena *arena, u64 mark);
function u64 arena_pos(Arena *arena);
function void *arena_push(Arena *arena, u64 size, u64 align, b32 clear);

/* arena push helpers */
#define push_array_no_zero_aligned(a, T, c, align) (T *)arena_push((a), sizeof(T)*(c), (align), (0))
#define push_array_aligned(a, T, c, align) (T *)arena_push((a), sizeof(T)*(c), (align), (1))
#define push_array_no_zero(a, T, c) push_array_no_zero_aligned(a, T, c, biggest(8, alignof(T)))
#define push_array(a, T, c) push_array_aligned(a, T, c, biggest(8, alignof(T)))

/* temp arenas */
typedef struct {
    Arena *arena;
    u64 mark;
} Temp_Arena;
function Temp_Arena begin_temp(Arena *arena);
function void end_temp(Temp_Arena arena);



/*== Strings ============================================*/


/* base string types */

typedef struct {
    u8 *ptr;
    u64 length;
} String8;

typedef struct {
    u16 *ptr;
    u64 length;
} String16;

typedef struct {
    u32 *ptr;
    u64 length;
} String32;

/* char operations */
function b32 char_is_space(u8 c);
function b32 char_is_upper(u8 c);
function b32 char_is_lower(u8 c);
function b32 char_is_alpha(u8 c);
function b32 char_is_slash(u8 c);
function b32 char_is_digit(u8 c, u32 base);
function u8 lower_from_char(u8 c);
function u8 upper_from_char(u8 c);
function u8 correct_slash_from_char(u8 c);

/* string length */
function u64 cstring8_length(u8 *str);
function u64 cstring16_length(u16 *str);
function u64 cstring32_length(u32 *str);

/* string constructors */
#define str8_lit(s) str8((u8*)(s), sizeof(s)-1)
#define str8_lit_comp(s) {(u8*)(s), sizeof(s)-1,}
#define str8_lit_cstr(s) str8((u8*)(s), sizeof(s))
#define str8_varg(s) (int)((s).length), ((s).ptr)

function String8  str8(u8 *str, u64 size);
function String16 str16(u16 *str, u64 size);
function String32 str32(u32 *str, u64 size);

function String8  str8_range(u8 *first, u8 *opl);
function String16 str16_range(u16 *first, u16 *opl);
function String32 str32_range(u32 *first, u32 *opl);

function String8  str8_cstring(char *c);
function String16 str16_cstring(u16 *c);
function String32 str32_cstring(u32 *c);

/* string formatting and copying */
function String8 str8_copy(Arena *arena, String8 str);
function String8 str8_cat(Arena *arena, String8 str1, String8 str2);
function String8 str8fv(Arena *arena, char *fmt, va_list args);
function String8 str8f(Arena *arena, char *fmt, ...);

/* string matching */
typedef enum {
    String_Match_Flag_Length_Insensitive = (1<<0),
    String_Match_Flag_Case_Insensitive   = (1<<1),
    String_Match_Flag_Slash_Insensitive  = (1<<2),
} String_Match_Flags;
#define str8_match_lit(a_lit, b, flags)   str8_match(str8_lit(a_lit), (b), (flags))
#define str8_match_cstr(a_cstr, b, flags) str8_match(str8_cstring(a_lit), (b), (flags))
function b32 str8_match(String8 a, String8 b, String_Match_Flags flags);
function u64 str8_find_needle(String8 string, u64 start_pos, String8 needle, String_Match_Flags flags);
#define str8_ends_with(string, end, flags) str8_match(str8_suffix((string), (end).length), (end), (flags))
#define str8_starts_with(string, start, flags) str8_match(str8_prefix((string), (start).size), (start), (flags))

/* string slicing */
function String8 str8_substr(String8 str, Range_U64 range);
function String8 str8_prefix(String8 str, u64 amount); // from the left
function String8 str8_chop(String8 str, u64 amount); // from the right
function String8 str8_skip(String8 str, u64 amount); // from the left
function String8 str8_suffix(String8 str, u64 amount); // from the right
function String8 str8_chop_line_and_skip(String8 *str);
function String8 str8_skip_and_chop_whitespace(String8 str);
function String8 str8_skip_comments(String8 str);
function String8 str8_skip_and_chop_slashes(String8 str);

/* file and path helpers */
typedef enum {
    Path_Style_None,
    Path_Style_Relative,
    Path_Style_Windows_Absolute,
    Path_Style_Unix_Absolute,
#if OS_WINDOWS
    Path_Style_SystemAbsolute = Path_Style_Windows_Absolute,
#elif OS_LINUX
    Path_Style_SystemAbsolute = Path_Style_Unix_Absolute,
#else
    Path_Style_SystemAbsolute = Path_Style_None,
#endif
} Path_Style;

function String8 str8_chop_last_slash(String8 str);
function String8 str8_skip_last_slash(String8 str);
function String8 str8_chop_last_dot(String8 str);
function String8 str8_skip_last_dot(String8 str);
function String8 str8_path_convert_slashes(String8 *str, Path_Style style);


/* string arrays and lists */

typedef struct String8_Node String8_Node;

struct String8_Node {
    String8_Node *next;
    String8 string;
};

typedef struct {
    String8_Node *first;
    String8_Node *last;
    u64 node_count;
    u64 total_size;
} String8_List;

/* push new string */
function String8_Node *str8_list_push(Arena *arena, String8_List *list, String8 string);
function String8_Node *str8_list_push_front(Arena *arena, String8_List *list, String8 string);
function String8_Node *str8_list_pushf(Arena *arena, String8_List *list, char *fmt, ...);
function String8_Node *str8_list_push_frontf(Arena *arena, String8_List *list, char *fmt, ...);

/* push node */
function String8_Node *str8_list_push_node(String8_List *list, String8_Node *node);
function String8_Node *str8_list_push_node_front(String8_List *list, String8_Node *node);

/* split a string into a list */
function String8_List str8_split(Arena *arena, String8 string, String8 separator);

/* joining lists */
typedef struct {
    String8 prefix;
    String8 separator;
    String8 postfix;
} String8_Join;

/* concatenates the nodes of a list into a single string */
function String8 str8_list_join(Arena *arena, String8_List *list, String8_Join *optional_params);



/*== Logging ============================================*/

typedef enum {
    Log_Level_Info,
    Log_Level_Error,

    Log_Level_Count,
} Log_Level;

typedef struct Log_Message Log_Message;
struct Log_Message {
    Log_Message *next;
    String8 msg;
    Log_Level level;
};

typedef struct Log_Frame Log_Frame;
struct Log_Frame {
    Log_Frame *next;
    Log_Message *first;
    Log_Message *last;
    u64 arena_mark;
};

typedef struct {
    Arena *arena;
    Log_Frame *top_frame;
} Log_Context;

function void log_frame_begin(void);
function String8 log_frame_peek(Arena *arena, i32 mask);
function String8 log_frame_end(Arena *arena, i32 mask);
function void log_frame_end_and_print(Arena *scratch, i32 mask);

function void log_emit(Log_Level level, String8 msg);
function void log_emitf(Log_Level level, char *fmt, ...);

#define log_error(msg)          log_emit(Log_Level_Error, msg)
#define log_errorf(msg, ...)    log_emitf(Log_Level_Error, msg, ##__VA_ARGS__)
#define log_info(msg)           log_emit(Log_Level_Info, msg)
#define log_infof(msg, ...)     log_emitf(Log_Level_Info, msg, ##__VA_ARGS__)



/*== Profiling ==========================================*/

/* profile block */
typedef struct {
    String8 label;

    /* accumulated data */
    u64 tsc_elapsed;
    u64 tsc_elapsed_children;
    u64 hit_count;

    /* data per hit */
    u64 start_tsc;
    u64 parent_index;
    u64 block_index;

} Profile_Block;

function void profile_block_begin(String8 label, u64 block_index);
function void profile_block_end(void);
#define profile_scope(label) defer_loop(profile_block_begin(str8_lit(label), __COUNTER__ + 1), profile_block_end())

/* profiler */
typedef struct {
    Profile_Block blocks[4096];
    u64 current_parent;
    u64 current_block;
    u64 start_tsc;
    u64 end_tsc;
} Profiler;
function void profiler_start(void);
function void profiler_end_and_print(void);



/*== Threads ============================================*/

typedef struct {
    u64 uint64[1];
} Thread;

typedef struct {
    u64 uint64[1];
} Mutex;

typedef struct {
    u64 uint64[1];
} RW_Mutex;

typedef struct {
    u64 uint64[1];
} Condition;

typedef struct {
    u64 uint64[1];
} Semaphore;

typedef struct {
    u64 uint64[1];
} Barrier;

typedef void Thread_Entry_Point_Func(void *params);

/* implemented per OS */
function Thread launch_thread(Thread_Entry_Point_Func *f, void *params);

/* implemented per OS */
/* waits indefinitely until the thread returns */
function void wait_join_thread(Thread thread);

/* implemented per OS */
function Mutex mutex_alloc(void);
function void mutex_free(Mutex mutex);
function void mutex_lock(Mutex mutex);
function void mutex_unlock(Mutex mutex);

/* scope macros */
#define mutex_scope(m) defer_loop(mutex_lock(m), mutex_unlock(m))
#define rw_mutex_scope(m, w) defer_loop(rw_mutex_lock((m), (w)), rw_mutex_unlock((m), (w)))
#define mutex_scope_read(m) defer_loop(rw_mutex_lock_read(m), rw_mutex_unlock_read(m))
#define mutex_scope_write(m) defer_loop(rw_mutex_lock_write(m), rw_mutex_unlock_write(m))

/* implemented per OS */
function RW_Mutex rw_mutex_alloc(void);
function void rw_mutex_free(RW_Mutex mutex);
function void rw_mutex_lock(RW_Mutex mutex, b32 write_mode);
function void rw_mutex_unlock(RW_Mutex mutex, b32 write_mode);
#define rw_mutex_lock_read(m)    rw_mutex_lock((m), false)
#define rw_mutex_lock_write(m)   rw_mutex_lock((m), true)
#define rw_mutex_unlock_read(m)  rw_mutex_unlock((m), false)
#define rw_mutex_unlock_write(m) rw_mutex_unlock((m), true)

/* implemented per OS */
function Condition cond_var_alloc(void);
function void cond_var_free(Condition cv);
/* blocks until signal or timeout. returns false on timeout, true on signal */
function b32 cond_var_wait(Condition cv, Mutex mutex, u64 timeout_us);
function b32 cond_var_rw_wait(Condition cv, RW_Mutex mutex, b32 write_mode, u64 timeout_us);
#define cond_var_wait_rw_read(cv, m, t)  cond_var_rw_wait((cv), (m), false, (t))
#define cond_var_wait_rw_write(cv, m, t) cond_var_rw_wait((cv), (m), true, (t))
function void cond_var_signal(Condition cv);
function void cond_var_broadcast(Condition cv);

/* implemented per OS */
function Semaphore semaphore_alloc(u32 initial_count, u32 max_count);
function void semaphore_free(Semaphore sem);
function b32 semaphore_take(Semaphore sem, u64 timeout_us);
function void semaphore_release(Semaphore sem);

/* implemented per OS */
function Barrier barrier_alloc(u32 count);
function void barrier_free(Barrier barrier);
function void barrier_wait(Barrier barrier);




/*== Thread Context =====================================*/

typedef struct {
    u64 lane_index;
    u64 lane_count;
    Barrier barrier;
    u64 *broadcast;
} Lane_Context;

typedef struct {
    Arena *arena;
    Arena *arenas[2];
    String8 name;
    Log_Context log;
    Lane_Context lane_ctx;
    Rand_State rand_state;
    Profiler profiler;
} Thread_Context;

typedef struct {
    String8 name;
    u64 scratch_size;
    u64 log_size;
} Thread_Context_Params;

function Thread_Context *tctx_init_(Arena *arena, String8 name, Thread_Context_Params *params);
#define tctx_init(arena, name, ...) tctx_init_(arena, name, &(Thread_Context_Params){ __VA_ARGS__ })

/* set the thread_static context pointer */
function void tctx_select(Thread_Context *ctx);

/* grab context through function call to avoid caching during async operations */
function Thread_Context *tctx_selected(void);

function Arena *tctx_get_scratch(Arena **conflicts, i32 count);
#define begin_scratch(arena, count) begin_temp(tctx_get_scratch(arena, count))
#define release_scratch(scratch) end_temp(scratch)

/* take the memory from broadcast_ptr and copy it out to other threads */
function void tctx_lane_barrier_wait(void *broadcast_ptr, u64 broadcast_size, u64 broadcast_src_lane_idx);

/* lane helpers */
#define lane_index() tctx_selected()->lane_ctx.lane_index
#define lane_count() tctx_selected()->lane_ctx.lane_count
#define lane_from_task_idx(idx) ((idx)%lane_count())
#define lane_sync() tctx_lane_barrier_wait(0, 0, 0)
#define lane_sync_u64(ptr, src_idx) tctx_lane_barrier_wait((ptr), sizeof(*(ptr)), (src_idx))
#define lane_range(count) m_range_from_n_index_m_count(lane_index(), lane_count(), (count))



/*== System API =========================================*/
/* implemented per OS */

typedef struct {
    String8 machine_name;
    u64 page_size;
    u64 large_page_size;
    u64 allocation_granularity;
    u32 logical_processor_count;
    u64 cpu_frequency;
} System_Info;

/* implemented per OS */
function System_Info *get_system_info(void);

/* implemented per OS */
/* wall clock time */
function u64 now_time_us(void);

/* implemented per OS */
/* wall clock time since start of program */
function u64 time_elapsed_us(void);

/* implemented per OS */
/* unix wall clock time */
function u64 now_time_unix(void);

/* implemented per OS */
function void sleep_ms(u32 ms);

/* reads an entire file into a string */
// TODO: make this os specific and add more features
function String8 read_entire_file(Arena *arena, String8 filename);



/*== Entry Point Caller =================================*/

typedef struct {
    String8 exe_name;
    String8 *args;
    u64 args_count;
} Cmd_Line;

/* called by the platform */
void entry_point_caller(Arena *arena, i32 argc, char **argv);

/* filled out by the application */
void entry_point(Cmd_Line cmdline);

/* filled out by the application */
void async_entry_point(void *thread_params);

#endif // CORELAYER_H

