/*== System Globals =====================================*/

global thread_static Thread_Context *tctx;



/*== Math ===============================================*/

function f32 f32_lerp(f32 a, f32 b, f32 t) {
    return a + (a + b)*t;
}

function u32 u32_lerp(u32 a, u32 b, f32 t) {
    return a + (u32)((a + b)*t);
}

/* evenly distributes m_count into n_count ranges and returns the n_idx'th range */
function Range_U64 m_range_from_n_index_m_count(u64 n_idx, u64 n_count, u64 m_count) {
    Range_U64 result = {0};
    u64 range_size_per_lane_rounded_down = m_count/n_count;
    u64 leftover_count = m_count - range_size_per_lane_rounded_down*n_count;
    u64 leftover_count_before_this_lane = smallest(n_idx, leftover_count);
    u64 lane_range_min = n_idx*range_size_per_lane_rounded_down + leftover_count_before_this_lane;
    u64 lane_range_min__clamped = smallest(lane_range_min, m_count);
    u64 lane_range_opl = lane_range_min__clamped + range_size_per_lane_rounded_down + ((n_idx < leftover_count) ? 1 : 0);
    u64 lane_range_opl__clamped = smallest(lane_range_opl, m_count);
    result.min = lane_range_min__clamped;
    result.max = lane_range_opl__clamped;
    return result;
}

/* vector constructors */
function Vec2f32 vec2(f32 x, f32 y) {
    Vec2f32 result = {.x = x, .y = y};
    return result;
}
function Vec3f32 vec3(f32 x, f32 y, f32 z) {
    Vec3f32 result = {.x = x, .y = y, .z = z};
    return result;
}
function Vec4f32 vec4(f32 x, f32 y, f32 z, f32 w) {
    Vec4f32 result = {.x = x, .y = y, .z = z, .w = w};
    return result;
}

/* range constructors */
function Range_f32 range_f32(f32 min, f32 max) {
    Range_f32 result = {.min = min, .max = max};
    return result;
}

/* vec3 operations */
function Vec3f32 vec3_add(Vec3f32 a, Vec3f32 b) {
    Vec3f32 result = { .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z };
    return result;
}
function Vec3f32 vec3_sub(Vec3f32 a, Vec3f32 b) {
    Vec3f32 result = { .x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z };
    return result;
}
function Vec3f32 vec3_mix(Vec3f32 a, Vec3f32 b) {
    Vec3f32 result = { .x = a.x*b.x, .y = a.y*b.y, .z = a.z*b.z };
    return result;
}
function Vec3f32 vec3_scale(Vec3f32 v, f32 s) {
    Vec3f32 result = { .x = v.x*s, .y = v.y*s, .z = v.z*s };
    return result;
}
function Vec3f32 vec3_negate(Vec3f32 v) {
    Vec3f32 result = { .x = -v.x, .y = -v.y, .z = -v.z };
    return result;
}
function Vec3f32 vec3_absf(Vec3f32 v) {
    Vec3f32 result = {.x = fabsf(v.x), .y = fabsf(v.y), .z = fabsf(v.z)};
    return result;
}
function Vec3f32 vec3_lerp(Vec3f32 a, Vec3f32 b, f32 t) {
    Vec3f32 lower = vec3_scale(a, 1.0f - t);
    Vec3f32 upper = vec3_scale(b, t);
    Vec3f32 result = vec3_add(lower, upper);
    return result;
}
function f32 vec3_dot(Vec3f32 a, Vec3f32 b) {
    f32 result = a.x*b.x + a.y*b.y + a.z*b.z;
    return result;
}
function Vec3f32 vec3_cross(Vec3f32 a, Vec3f32 b) {
    Vec3f32 result = {
        .x = a.y*b.z - a.z*b.y,
        .y = a.z*b.x - a.x*b.z,
        .z = a.x*b.y - a.y*b.x,
    };
    return result;
}
function f32 vec3_length(Vec3f32 v) {
    f32 result = sqrtf(vec3_dot(v, v));
    return result;
}
function Vec3f32 vec3_norm(Vec3f32 v) {
    Vec3f32 result = {0};
    f32 scale = vec3_length(v);
    if (scale > -0.001f || 0.001f < scale)
        result = vec3_scale(v, 1.0f/scale);
    return result;
}
function b32 vec3_near_zero(Vec3f32 v) {
    b32 result = false;
    if (fabs(v.x) < 1e-8 && fabs(v.y) < 1e-8 && fabs(v.z) < 1e-8) {
        result = true;
    }
    return result;
}

/* assumes unit normal vector */
function Vec3f32 vec3_reflect(Vec3f32 v, Vec3f32 n) {
    Vec3f32 projected = vec3_scale(n, -2.0f*vec3_dot(v,n));
    Vec3f32 result = vec3_add(v, projected);
    return result;
}
/* n1 is the rafractive index of the initial medium */
function Vec3f32 vec3_refract(Vec3f32 v, Vec3f32 n, f32 n1_over_n2) {
    Vec3f32 nv = vec3_negate(v);
    f32 cos_theta = vec3_dot(nv, n);
    cos_theta = smallest(cos_theta, 1.0f);
    Vec3f32 vn = vec3_scale(n, cos_theta);
    Vec3f32 perp = vec3_add(v, vn);
    perp = vec3_scale(perp, n1_over_n2);
    f32 perp_length_squared = vec3_dot(perp, perp);
    f32 scale = -sqrtf(fabsf(1.0f - perp_length_squared));
    Vec3f32 parallel = vec3_scale(n, scale);
    Vec3f32 result = vec3_add(perp, parallel);
    return result;
}

/* color operations */
function Vec3f32 linear_from_srgb(Vec3f32 srgb) {
    Vec3f32 result = {
        .r = srgb.r < 0.0404482362771082f ? srgb.r/12.92f : pow_f32((srgb.r + 0.055f)/1.055f, 2.4f),
        .g = srgb.g < 0.0404482362771082f ? srgb.g/12.92f : pow_f32((srgb.g + 0.055f)/1.055f, 2.4f),
        .b = srgb.b < 0.0404482362771082f ? srgb.b/12.92f : pow_f32((srgb.b + 0.055f)/1.055f, 2.4f),
    };
    return result;
}
function Vec3f32 srgb_from_linear(Vec3f32 linear) {
    Vec3f32 result = {
        .r = (0 <= linear.r && linear.r < 0.00313066844250063) ? linear.r*12.92f : 1.05f*pow_f32(linear.r, 1.0f/2.4f) - 0.055f,
        .g = (0 <= linear.g && linear.g < 0.00313066844250063) ? linear.g*12.92f : 1.05f*pow_f32(linear.g, 1.0f/2.4f) - 0.055f,
        .b = (0 <= linear.b && linear.b < 0.00313066844250063) ? linear.b*12.92f : 1.05f*pow_f32(linear.b, 1.0f/2.4f) - 0.055f,
    };
    return result;
}

/* random vectors */
function Vec3f32 rand_unit_vec3(void) {
    Vec3f32 result = rand_unit_vec3_r(&tctx_selected()->rand_state);
    return result;
}
function Vec3f32 rand_unit_vec3_r(Rand_State *rng) {
    Vec3f32 result = {0};
    for (;;) {
        /* components are between -1.0 and 1.0 */
        Vec3f32 v = {
            .x = rand_f32_r(rng)*2.0f - 1.0f,
            .y = rand_f32_r(rng)*2.0f - 1.0f,
            .z = rand_f32_r(rng)*2.0f - 1.0f,
        };

        /* avoid subnormal floats */
        f32 length_squared = vec3_dot(v, v);
        if (1e-45f < length_squared && length_squared <= 1.0f) {
            result = vec3_scale(v, 1.0f/sqrtf(length_squared));
            break;
        }
    }
    return result;
}
function Vec3f32 rand_vec3_in_unit_disk(void) {
    Vec3f32 result = rand_vec3_in_unit_disk_r(&tctx_selected()->rand_state);
    return result;
}
function Vec3f32 rand_vec3_in_unit_disk_r(Rand_State *rng) {
    Vec3f32 result = {0};
    for (;;) {
        /* components are between -1.0 and 1.0 */
        Vec3f32 v = {
            .x = rand_f32_r(rng)*2.0f - 1.0f,
            .y = rand_f32_r(rng)*2.0f - 1.0f,
            .z = rand_f32_r(rng)*2.0f - 1.0f,
        };

        /* vector length should be less than 1.0 */
        f32 length_squared = vec3_dot(v, v);
        if (length_squared < 1.0f) {
            result = v;
            break;
        }
    }
    return result;
}

/* random color */
function Vec3f32 rand_color(void) {
    Vec3f32 result = { .r = rand_f32(), .g = rand_f32(), .b = rand_f32() };
    return result;
}


/* quaternions */
function Quaternion quat(Vec3f32 axis, f32 rads) {
    Quaternion result = {
        .a = cosf(0.5f*rads),
        .b = axis.x*sinf(0.5f*rads),
        .c = axis.y*sinf(0.5f*rads),
        .d = axis.z*sinf(0.5f*rads),
    };
    return result;
}
function Quaternion quat_conjugate(Quaternion q) {
    Quaternion result = { .a =  q.a, .b = -q.b, .c = -q.c, .d = -q.d };
    return result;
}
function Quaternion quat_inverse(Quaternion q, b32 *exists) {
    Quaternion result = {0};
    *exists = false;
    float denom = q.a*q.a + q.b*q.b + q.c*q.c + q.d*q.d;
    if (denom > 1e-8f) {
        *exists = true;
        result.a =  q.a/denom;
        result.b = -q.b/denom;
        result.c = -q.c/denom;
        result.d = -q.d/denom;
    }
    return result;
}
function Quaternion quat_mul(Quaternion q, Quaternion p) {
    Quaternion result = {0};
    result.a = q.a*p.a - q.b*p.b - q.c*p.c - q.d*p.d;
    result.b = q.a*p.b + q.b*p.a + q.c*p.d - q.d*p.c;
    result.c = q.a*p.c - q.b*p.d + q.c*p.a + q.d*p.b;
    result.d = q.a*p.d + q.b*p.c - q.c*p.b + q.d*p.a;
    return result;
}
function Vec3f32 rotate(Quaternion q, Vec3f32 p) {
    Quaternion v = { .b = p.x, .c = p.y, .d = p.z };
    b32 exists;
    Quaternion qi = quat_inverse(q, &exists);
    if (exists) {
        Quaternion vqi = quat_mul(v, qi);
        Quaternion res = quat_mul(q, vqi);
        p.x = res.b;
        p.y = res.c;
        p.z = res.d;
    }
    return p;
}



/*== Pseudo RNG =========================================*/

/* seeding functions */
function void rand_seed(u64 initstate, u64 initseq) {
    rand_seed_r(&tctx_selected()->rand_state, initstate, initseq);
}
function void rand_seed_r(Rand_State* rng, u64 initstate, u64 initseq) {
    rng->state = 0u;
    rng->inc = (initseq << 1u) | 1u;
    rand_u32_r(rng);
    rng->state += initstate;
    rand_u32_r(rng);
}

/* generate random ints */
function u32 rand_u32(void) {
    return rand_u32_r(&tctx_selected()->rand_state);
}
function u32 rand_u32_r(Rand_State *rng) {
    u64 old_state = rng->state;
    rng->state = old_state*6364136223846793005ULL + rng->inc;
    u32 xorshifted = (u32)(((old_state >> 18u) ^ old_state) >> 27u);
    u32 rot = old_state >> 59u;
    u32 result = ((xorshifted >> rot) | (xorshifted << ((-rot) & 31)));
    return result;
}
function i32 rand_i32_in_range(i32 min, i32 max) {
    return rand_i32_in_range_r(&tctx_selected()->rand_state, min, max);
}
function i32 rand_i32_in_range_r(Rand_State *rng, i32 min, i32 max) {
    i32 span = max - min;
    i32 result = min + ((i32)rand_u32_r(rng) % span);
    return result;
}

/* generated random float [0.0, 1.0] */
function f32 rand_f32(void) {
    f32 result = rand_f32_r(&tctx_selected()->rand_state);
    return result;
}
function f32 rand_f32_r(Rand_State *rng) {
    f32 result = (f32)rand_u32_r(rng)/max_u32;
    return result;
}



/*== Arenas =============================================*/

function Arena arena_init(void *base, u64 cap) {
    Arena result = {0};
    result.base = base;
    result.cap = cap;
    return result;
}

function Arena arena_alloc(Arena *arena, u64 cap) {
    Arena result = {0};
    void *base = push_array(arena, u8, cap);
    if (base) {
        result = arena_init(base, cap);
    }
    return result;
}

function void arena_reset(Arena *arena) {
    arena->pos = 0;
}

function void arena_rewind(Arena *arena, u64 pos) {
    arena->pos = pos;
}

function u64 arena_pos(Arena *arena) {
    return arena->pos;
}

function void *arena_push(Arena *arena, u64 size, u64 align, b32 clear) {
    void *result = NULL;
    if (arena) {
        u64 aligned_pos = align_pow2(arena->pos + size, align);
        if (aligned_pos < arena->cap) {
            result = (char *)arena->base + arena->pos;
            arena->pos = aligned_pos;
            
            if (clear) memset(result, 0, size);
        } else {
            // TODO: add more arena state to log more context about allocation
            log_error(str8_lit("Arena out of memory!"));
        }
    }
    return result;
}

function Temp_Arena begin_temp(Arena *arena) {
    Temp_Arena result = {0};
    result.arena = arena;
    result.saved_pos = arena->pos;
    result.saved_temp_count = arena->temp_count++;
    return result;
}

function void end_temp(Temp_Arena *temp) {
    temp->arena->temp_count -= 1;
    debug_assert(temp->saved_temp_count == temp->arena->temp_count);
    temp->arena->pos = temp->saved_pos;
    temp->arena = 0;
}



/*== Strings ============================================*/

/* char operations */
function b32 char_is_space(u8 c) {
    return (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v');
}
function b32 char_is_upper(u8 c) {
    return ('A' <= c && c <= 'Z');
}
function b32 char_is_lower(u8 c) {
    return ('a' <= c && c <= 'z');
}
function b32 char_is_alpha(u8 c) {
    return (char_is_upper(c) || char_is_lower(c));
}
function b32 char_is_slash(u8 c) {
    return (c == '/' || c == '\\');
}
function b32 char_is_digit(u8 c, u32 base) {
    b32 result = false;
    if (0 < base && base <= 16) {
        u8 val = integer_symbol_reverse[c];
        if (val < base)
            result = true;
    }
    return result;
}
function u8 lower_from_char(u8 c) {
    return char_is_upper(c) ? c + ('a' - 'A') : c;
}
function u8 upper_from_char(u8 c) {
    return char_is_lower(c) ? c + ('A' - 'a') : c;
}
function u8 correct_slash_from_char(u8 c) {
    return char_is_slash(c) ? '/' : c;
}

/* string length */
function u64 cstring8_length(u8 *str) {
    u64 length = 0;
    if (str) {
        u8 *p = str;
        for (;*p != 0; p += 1);
        length = (u64)(p - str);
    }
    return length;
}
function u64 cstring16_length(u16 *str) {
    u64 length = 0;
    if (str) {
        u16 *p = str;
        for (;*p != 0; p += 1);
        length = (u64)(p - str);
    }
    return length;
}
function u64 cstring32_length(u32 *str) {
    u64 length = 0;
    if (str) {
        u32 *p = str;
        for (;*p != 0; p += 1);
        length = (u64)(p - str);
    }
    return length;
}

/* string constructors */
function String8 str8(u8 *str, u64 size) {
    String8 result = {str, size};
    return result;
}
function String16 str16(u16 *str, u64 size) {
    String16 result = {str, size};
    return result;
}
function String32 str32(u32 *str, u64 size) {
    String32 result = {str, size};
    return result;
}
function String8 str8_range(u8 *first, u8 *opl) {
    String8 result = {first, (u64)(opl - first)};
    return result;
}
function String16 str16_range(u16 *first, u16 *opl) {
    String16 result = {first, (u64)(opl - first)};
    return result;
}
function String32 str32_range(u32 *first, u32 *opl) {
    String32 result = {first, (u64)(opl - first)};
    return result;
}
function String8 str8_cstring(char *str) {
    String8 result = {(u8*)str, cstring8_length((u8*)str)};
    return result;
}
function String16 str16_cstring(u16 *str) {
    String16 result = {str, cstring16_length(str)};
    return result;
}
function String32 str32_cstring(u32 *str) {
    String32 result = {str, cstring32_length(str)};
    return result;
}

/* string formatting and copying */
function String8 str8_copy(Arena *arena, String8 str) {
    String8 result = {0};
    if (arena) {
        u8 *copy = push_array_no_zero(arena, u8, str.length + 1);
        if (copy) {
            mem_copy(copy, str.ptr, str.length);
            copy[str.length] = 0;
            result.ptr = copy;
            result.length = str.length;
        }
    }
    return result;
}
function String8 str8_cat(Arena *arena, String8 str1, String8 str2) {
    String8 result = {0};
    if (arena) {
        u64 length = str1.length + str2.length;
        u8 *cat = push_array_no_zero(arena, u8, length + 1);
        if (cat) {
            result.ptr = cat;
            result.length = length;
            mem_copy(cat, str1.ptr, str1.length);
            cat += str1.length;
            mem_copy(cat, str2.ptr, str2.length);
            cat += str2.length;
            *cat = 0;
        }
    }
    return result;
}
function String8 str8fv(Arena *arena, char *fmt, va_list args) {
    va_list args2;
    va_copy(args2, args);
    u32 size = vsnprintf(0, 0, fmt, args) + 1;
    String8 result = {0};
    u8 *string = push_array_no_zero(arena, u8, size);
    if (string) {
        result.ptr = string;
        result.length = vsnprintf((char*)string, size, fmt, args2);
        result.ptr[result.length] = 0;
    }
    va_end(args2);
    return result;
}
function String8 str8f(Arena *arena, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String8 result = str8fv(arena, fmt, args);
    va_end(args);
    return result;
}

/* string matching */
function b32 str8_match(String8 a, String8 b, String_Match_Flags flags) {
    b32 result = false;
    if (a.length == b.length && flags == 0) {
        result = mem_match(a.ptr, b.ptr, a.length);
    } else if (a.length == b.length || flags & String_Match_Flag_Length_Insensitive) {
        u64 length = smallest(a.length, b.length);
        result = true;
        for (u64 i = 0; i < length; i += 1) {
            u8 at = a.ptr[i];
            u8 bt = b.ptr[i];
            if (flags & String_Match_Flag_Case_Insensitive) {
                at = lower_from_char(at);
                bt = lower_from_char(bt);
            }
            if (flags & String_Match_Flag_Slash_Insensitive) {
                at = correct_slash_from_char(at);
                bt = correct_slash_from_char(bt);
            }
            if (at != bt) {
                result = false;
                break;
            }
        }
    }
    return result;
}
function u64 str8_find_needle(String8 string, u64 start_pos, String8 needle, String_Match_Flags flags) {
    u8 *p = string.ptr + start_pos;
    u64 stop_offset = biggest(string.length + 1, needle.length) - needle.length;
    u8 *stop_p = string.ptr + stop_offset;

    if (needle.length > 0) {

        u8 *string_opl = string.ptr + string.length;
        u8 n = needle.ptr[0];
        String8 eedle = str8_skip(needle, 1);

        String_Match_Flags adjusted_flags = flags | String_Match_Flag_Length_Insensitive;

        if (adjusted_flags & String_Match_Flag_Case_Insensitive)
            n = lower_from_char(n);

        if (adjusted_flags & String_Match_Flag_Slash_Insensitive)
            n = correct_slash_from_char(n);

        for (;p < stop_p; p += 1) {
            u8 h = *p;

            if (adjusted_flags & String_Match_Flag_Case_Insensitive)
                h = lower_from_char(h);

            if (adjusted_flags & String_Match_Flag_Slash_Insensitive)
                h = correct_slash_from_char(h);

            if (h == n) {
                if (str8_match(str8_range(p+1, string_opl), eedle, adjusted_flags))
                    break;
            }
        }
    }
    u64 result = string.length;
    if (p < stop_p) {
        result = (u64)(p - string.ptr);
    }
    return result;
}

/* string slicing */
function String8 str8_substr(String8 str, Range_U64 range) {
    range.min = smallest(range.min, str.length);
    range.max = smallest(range.max, str.length);
    str.ptr += range.min;
    str.length = range_size(range);
    return str;
}
function String8 str8_prefix(String8 str, u64 amount) {
    str.length = smallest(amount, str.length);
    return str;
}
function String8 str8_chop(String8 str, u64 amount) {
    amount = smallest(str.length, amount);
    str.length -= amount;
    return str;
}
function String8 str8_suffix(String8 str, u64 amount) {
    amount = smallest(str.length, amount);
    str.ptr = (str.ptr + str.length) - amount;
    str.length = amount;
    return str;
}
function String8 str8_skip(String8 str, u64 amount) {
    amount = smallest(str.length, amount);
    str.ptr += amount;
    str.length -= amount;
    return str;
}
function String8 str8_chop_line_and_skip(String8 *str) {
    u64 new_line_pos = str8_find_needle(*str, 0, str8_lit("\n"), 0);
    String8 result = str8_prefix(*str, new_line_pos);
    if (str8_ends_with(result, str8_lit("\r"), 0))
        result = str8_chop(result, 1);
    *str = str8_skip(*str, new_line_pos + 1);
    return result;
}
function String8 str8_skip_and_chop_whitespace(String8 str) {
    u8 *first = str.ptr;
    u8 *opl = first + str.length;
    for (;first < opl; first += 1) {
        if (!char_is_space(*first))
            break;
    }
    for (;opl > first;) {
        opl -= 1;
        if (!char_is_space(*opl)) {
            opl += 1;
            break;
        }
    }
    String8 result = str8_range(first, opl);
    return result;
}
function String8 str8_skip_comments(String8 str) {
    String8 result = {0};
    if (str.ptr[0] == '/') {
        if (str.ptr[1] == '/') {
            // skip single line
            u64 pos = str8_find_needle(str, 0, str8_lit("\n"), 0);
            result = str8_skip(str, pos);
        } else if (str.ptr[1] == '*') {
            /* skip multi-line */
            u64 pos = str8_find_needle(str, 0, str8_lit("*/"), 0);
            result = str8_skip(str, pos);
        }
    }
    return result;
}
function String8 str8_skip_and_chop_slashes(String8 str) {
    u8 *first = str.ptr;
    u8 *opl = first + str.length;
    for (; first < opl; first += 1) {
        if (!char_is_slash(*first))
            break;
    }
    for (; opl > first;) {
        opl -= 1;
        if (!char_is_slash(*opl)) {
            opl += 1;
            break;
        }
    }
    String8 result = str8_range(first, opl);
    return result;
}

/* file and path helpers */
function String8 str8_chop_last_slash(String8 str) {
    String8 result = {0};
    if (str.length > 0) {
        u8 *ptr = str.ptr + str.length - 1;
        for (; ptr >= str.ptr; ptr -= 1) {
            if (char_is_slash(*ptr))
                break;
        }
        if (ptr >= str.ptr) {
            result.length = (u64)(ptr - str.ptr);
            result.ptr = str.ptr;
        } 
    }
    return result;
}
function String8 str8_skip_last_slash(String8 str) {
    String8 result = {0};
    if (str.length > 0) {
        u8 *ptr = str.ptr + str.length - 1;
        for (; ptr >= str.ptr; ptr -= 1) {
            if (char_is_slash(*ptr))
                break;
        }
        if (ptr >= str.ptr) {
            ptr += 1;
            result.length = (u64)(str.ptr + str.length - ptr);
            result.ptr = ptr;
        }
    }
    return result;
}
function String8 str8_chop_last_dot(String8 str) {
    String8 result = {0};
    if (str.length > 0) {
        u8 *ptr = str.ptr + str.length - 1;
        for (; ptr >= str.ptr; ptr -= 1) {
            if (*ptr == '.')
                break;
        }
        if (ptr >= str.ptr) {
            result.length = (u64)(ptr - str.ptr);
            result.ptr = str.ptr;
        } 
    }
    return result;
}
function String8 str8_skip_last_dot(String8 str) {
    String8 result = {0};
    if (str.length > 0) {
        u8 *ptr = str.ptr + str.length - 1;
        for (; ptr >= str.ptr; ptr -= 1) {
            if (*ptr == '.')
                break;
        }
        if (ptr >= str.ptr) {
            ptr += 1;
            result.length = (u64)(str.ptr + str.length - ptr);
            result.ptr = ptr;
        }
    }
    return result;
}
function String8 str8_path_convert_slashes(String8 *str, Path_Style style) {
    String8 result = {0};
    not_yet_implemented();
    return result;
}

/* splitting a string into a list */
function String8_List str8_split(Arena *arena, String8 string, String8 separator) {
    String8_List result = {0};
    not_yet_implemented();
    return result;
}

/* list builders */
function String8_Node *str8_list_push(Arena *arena, String8_List *list, String8 string) {
    String8_Node *node = push_struct(arena, String8_Node);
    if (node) {
        node->string = string;
        str8_list_push_node(list, node);
    }
    return node;
}
function String8_Node *str8_list_push_front(Arena *arena, String8_List *list, String8 string) {
    String8_Node *node = push_struct(arena, String8_Node);
    if (node) {
        node->string = string;
        str8_list_push_node_front(list, node);
    }
    return node;
}
function String8_Node *str8_list_pushf(Arena *arena, String8_List *list, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String8 string = str8fv(arena, fmt, args);
    String8_Node *result = str8_list_push(arena, list, string);
    va_end(args);
    return result;
}
function String8_Node *str8_list_push_frontf(Arena *arena, String8_List *list, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String8 string = str8fv(arena, fmt, args);
    String8_Node *result = str8_list_push_front(arena, list, string);
    va_end(args);
    return result;
}
function String8_Node *str8_list_push_node(String8_List *list, String8_Node *node) {
    debug_assert(node);
    sll_queue_push(list->first, list->last, node);
    list->node_count += 1;
    list->total_size += node->string.length;
    return node;
}
function String8_Node *str8_list_push_node_front(String8_List *list, String8_Node *node) {
    debug_assert(node);
    sll_queue_push_front(list->first, list->last, node);
    list->node_count += 1;
    list->total_size += node->string.length;
    return node;
}

/* joining a list into string */
function String8 str8_list_join(Arena *arena, String8_List *list, String8_Join *optional_params) {
    String8_Join join = {0};
    if (optional_params)
        mem_copy_struct(&join, optional_params);

    u64 separator_count = 0;
    if (list->node_count > 0)
        separator_count = list->node_count - 1;

    String8 result = {0};
    u64 size = join.prefix.length + join.postfix.length + separator_count*join.separator.length + list->total_size;
    u8 *ptr = push_array_no_zero(arena, u8, size + 1);
    if (ptr) {
        result.ptr = ptr;
        result.length = size;

        mem_copy(ptr, join.prefix.ptr, join.prefix.length);
        ptr += join.prefix.length;
        for (String8_Node *node = list->first; node != 0; node = node->next) {
            mem_copy(ptr, node->string.ptr, node->string.length);
            ptr += node->string.length;
            if (node->next != 0) {
                mem_copy(ptr, join.separator.ptr, join.separator.length);
                ptr += join.separator.length;
            }
        }
        mem_copy(ptr, join.postfix.ptr, join.postfix.length);
        ptr += join.postfix.length;
        *ptr = 0;
    } 
    return result;
}



/*== Logging ============================================*/

function void log_frame_begin(void) {
    Log_Context *log = &tctx->log;
    Log_Frame *frame = push_struct(&log->arena, Log_Frame);
    if (frame) {
        sll_stack_push(log->top_frame, frame);
        frame->arena_pos = arena_pos(&log->arena);
    }
}

function String8 log_frame_peek(Arena *arena, int mask) {
    String8 result = {0};
    Log_Frame *frame = tctx_selected()->log.top_frame;
    if (frame && arena) {
        Temp_Arena scratch = begin_scratch(arena, 1);
        String8_List list = {0};
        for (Log_Message *msg = frame->first; msg != 0; msg = msg->next) {
            if (mask == Log_Level_Info)
                str8_list_push(scratch.arena, &list, str8_lit("Info:  "));
            else if (mask == Log_Level_Error)
                str8_list_push(scratch.arena, &list, str8_lit("Error: "));
            str8_list_pushf(scratch.arena, &list, "%.*s\n", str8_varg(msg->msg));
        }
        result = str8_list_join(arena, &list, 0);
        release_scratch(&scratch);
    }
    return result;
}

function String8 log_frame_end(Arena *arena, int mask) {
    String8 result = {0};
    if (tctx && arena) {
        Log_Context *log = &tctx_selected()->log;
        Log_Frame *frame = log->top_frame;
        if (frame) {
            result = log_frame_peek(arena, mask);
            sll_stack_pop(log->top_frame);
            arena_rewind(&log->arena, frame->arena_pos);
        }
    }
    return result;
}

function void log_frame_end_and_print(Arena *arena, int mask) {
    String8 messages = log_frame_end(arena, mask);
    if (messages.length > 0) {
        fprintf(stderr, "%.*s", str8_varg(messages));
    }
}

function void log_emit(Log_Level level, String8 string) {
    Log_Context *log = &tctx_selected()->log;
    Log_Frame *frame = log->top_frame;
    if (frame) {
        Log_Message *message = push_struct(&log->arena, Log_Message);
        if (message) {
            String8 copy = str8_copy(&log->arena, string);
            message->msg = copy;
            message->level = level;
            sll_queue_push(frame->first, frame->last, message);
        }
    }
}

function void log_emitf(Log_Level level, char *fmt, ...) {
    char buffer[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    size_t len = strlen(buffer);
    uint8_t *cstr = push_array(&tctx->log.arena, uint8_t, len);
    memcpy(cstr, buffer, len);
    log_emit(level, (String8){.ptr = cstr, .length = len});
}


/*== Profiler ===========================================*/

function void profile_block_begin(String8 label, u64 block_index) {
    /* initialize block */
    Profile_Block *block = &tctx_selected()->profiler.blocks[block_index];
    block->label = label;
    block->parent_index = tctx_selected()->profiler.current_parent;
    block->block_index = block_index;

    /* update global state */
    tctx_selected()->profiler.current_block = block_index;
    tctx_selected()->profiler.current_parent = block_index;

    block->start_tsc = read_cpu_timestamp();
}
function void profile_block_end(void) {
    u64 block_index = tctx_selected()->profiler.current_block;
    Profile_Block *block = &tctx_selected()->profiler.blocks[block_index];
    debug_assert(block->block_index == block_index);

    /* submit profile block */
    u64 cycles_elapsed = read_cpu_timestamp() - block->start_tsc;
    block->tsc_elapsed += cycles_elapsed;
    block->hit_count += 1;

    tctx_selected()->profiler.blocks[block->parent_index].tsc_elapsed_children += cycles_elapsed;

    /* update global state */
    tctx_selected()->profiler.current_parent = block->parent_index;
    tctx_selected()->profiler.current_block = block->parent_index;
}

function void profiler_start(void) {
    tctx_selected()->profiler.start_tsc = read_cpu_timestamp();
}
function void profiler_end_and_print(void) {
    Profiler *prof = &tctx_selected()->profiler;
    u64 total_elapsed = read_cpu_timestamp() - prof->start_tsc;

    for (u32 i = 1; i < arrcount(prof->blocks); i += 1) {
        Profile_Block *block = &prof->blocks[i];
        if (block->tsc_elapsed) {
            u64 exclusive_elapsed = block->tsc_elapsed - block->tsc_elapsed_children;
            f32 exclusive_percent = 100.0f*exclusive_elapsed/total_elapsed;
            printf("Thread [%.*s] - %.*s: %zu cycles (%.2f%%)\n", str8_varg(tctx_selected()->name), str8_varg(block->label), exclusive_elapsed, exclusive_percent);
        }
    }
}



/*== Thread Context =====================================*/

function Thread_Context *tctx_init_(Arena *arena, Thread_Context_Params *params) {
    Thread_Context *result = push_struct(arena, Thread_Context);
    if (result) {

        result->name = params->name;

        /* initialize scratch arenas */
        size_t scratch_size = params->scratch_size;
        if (!scratch_size) {
            scratch_size = MiB(64);
        }

        /* initialize logging */
        size_t log_size = params->log_size;
        if (!log_size) {
            log_size = MiB(4);
        }

        result->arenas[0] = arena_alloc(arena, scratch_size);
        result->arenas[1] = arena_alloc(arena, scratch_size);
        result->log.arena = arena_alloc(arena, log_size);

        /* initialize static rng */
        result->rand_state.state = 0x853c49e6748fea9bULL;
        result->rand_state.inc   = 0xda3e39cb94b95bdbULL;
        rand_seed_r(&result->rand_state, now_time_us() ^ int_from_ptr(&printf), int_from_ptr(tctx));
    }
    return result;
}

function Arena *tctx_get_scratch(Arena *conflicts, int count) {
    Arena *result = 0;
    if (tctx) {
        for (int i = 0; i < arrcount(tctx->arenas); i += 1) {
            b32 has_conflict = false;
            for (int j = 0; j < count; j += 1) {
                if (&tctx->arenas[i] == &conflicts[j]) {
                    has_conflict = true;
                    break;
                }
            }
            if (!has_conflict) {
                result = &tctx->arenas[i];
                break;
            }
        }
    }
    return result;
}

/* select the thread_static ptr */
function void tctx_select(Thread_Context *ctx) {
    tctx = ctx;
}

/* return as a function call to avoid caching values during async operations */
function Thread_Context *tctx_selected(void) {
    return tctx;
}

/* take the memory from broadcast_ptr and copy it out to other threads */
function void tctx_lane_barrier_wait(void *broadcast_ptr, u64 broadcast_size, u64 broadcast_src_lane_idx) {

    /* TODO: decide what can be broadcasted */
    u64 broadcast_size_clamped = clamp_top(broadcast_size, sizeof(tctx->lane_ctx.broadcast[0]));

    if (broadcast_ptr && lane_index() == broadcast_src_lane_idx) {
        mem_copy(tctx->lane_ctx.broadcast, broadcast_ptr, broadcast_size_clamped);
    }

    /* wait for all lanes to catch up */
    barrier_wait(tctx->lane_ctx.barrier);

    if (broadcast_ptr && lane_index() != broadcast_src_lane_idx) {
        mem_copy(broadcast_ptr, tctx->lane_ctx.broadcast, broadcast_size_clamped);
    }

    if (broadcast_ptr) {
        barrier_wait(tctx->lane_ctx.barrier);
    }
}



function String8 read_entire_file(Arena *arena, String8 filename) {
    String8 result = {0};
    FILE *file = fopen((char*)filename.ptr, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        u64 size = ftell(file);
        fseek(file, 0, SEEK_SET);
        u8 *file_data = push_array_no_zero(arena, u8, size);
        if (file_data) {
            u64 size_read = fread(file_data, 1, size, file);
            result = str8(file_data, size_read);
        }
    }
    return result;
}



/*== Entry Point Caller =================================*/

void entry_point_caller(Arena *arena, i32 argc, char **argv) {

    /* parse arguments */
    Cmd_Line cmdline = {0};
    cmdline.exe_name = str8_cstring(argv[0]);
    cmdline.args = push_array(arena, String8, argc - 1);
    for (int i = 1; i < argc; i += 1) {
        cmdline.args[i-1] = str8_cstring(argv[i]);
        cmdline.args_count += 1;
    }

    entry_point(arena, cmdline);
}


#if OS_WINDOWS
#include "corelayer_win32.c"
#elif OS_LINUX
#include "corelayer_linux.c"
#endif

