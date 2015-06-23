#pragma version(1)
#pragma rs java_package_name(com.android.rs.kernellocalvariables)

float4 gColor = { 0.299f, 0.587f, 0.114f, 1.f };
uint4  gTrunc = { 0x1, 0x10, 0x100, 0x0 };

char char_global = 12;
uchar uchar_global = 234;
short short_global = -321;
ushort ushort_global = 432;
int int_global = 1234;
uint uint_global = 2345;
float float_global = 4.5f;
long long_global = -77777;
ulong ulong_global = 8888;
double double_global = -456.5f;

char2 char2_global = (char2){11, -22};
uchar2 uchar2_global = (uchar2){33, 44};
short2 short2_global = (short2){-555, 666};
ushort2 ushort2_global = (ushort2){777, 888};
int2 int2_global = (int2){999, -1111};
uint2 uint2_global = (uint2){2222, 3333};
float2 float2_global = (float2){4.5f, -5.0f};
long2 long2_global = (long2){-4444, 5555};
ulong2 ulong2_global = (ulong2){6666, 7777};
double2 double2_global = (double2){88.5f, -99.0f};

char3 char3_global = (char3){11, -22, -33};
uchar3 uchar3_global = (uchar3){33, 44, 55};
short3 short3_global = (short3){-555, 666, 777};
ushort3 ushort3_global = (ushort3){777, 888, 999};
int3 int3_global = (int3){999, -1111, 2222};
uint3 uint3_global = (uint3){2222, 3333, 4444};
float3 float3_global = (float3){4.5f, -5.0f, -6.5f};
long3 long3_global = (long3){-4444, 5555, 6666};
ulong3 ulong3_global = (ulong3){6666, 7777, 8888};
double3 double3_global = (double3){88.5f, -99.0f, 111.5f};

char4 char4_global = (char4){55, 11, -22, -33};
uchar4 uchar4_global = (uchar4){222, 33, 44, 55};
short4 short4_global = (short4){-444, -555, 666, 777};
ushort4 ushort4_global = (ushort4){666, 777, 888, 999};
int4 int4_global = (int4){888, 999, -1111, 2222};
uint4 uint4_global = (uint4){1111, 2222, 3333, 4444};
float4 float4_global = (float4){3.0f, 4.5f, -5.0f, -6.5f};
long4 long4_global = (long4){-3333, -4444, 5555, 6666};
ulong4 ulong4_global = (ulong4){5555, 6666, 7777, 8888};
double4 double4_global = (double4){-77.0f, 88.5f, -99.0f, 111.5f};

struct S
{
    uchar a, b, c, d;
};

static void truncate(uchar4 * result, float4 inf, uint4 ini)
{
    result->x = ini.x + inf.x;
    result->y = ini.y + inf.y;
    result->z = ini.z + inf.z;
    result->w = ini.w + inf.w;
    return;
}

void set_s(uchar4 in)
{
    struct S s;
    s.a = in.x;
    s.b = in.y;
    s.c = in.z;
    s.d = in.w;
    
    uchar2 t = in.s01 + in.s23;
    s.a = t.x + t.y;
    uchar4 result;
    truncate(&result, gColor, gTrunc);
    return;
}

uchar4 __attribute__((kernel)) simple_kernel(uchar4 in)
{
    set_s(in);
    uchar4 result;
    result.wzyx = in.xyzw;
    return result;
}

/*
 * Separate kernel for long2 because putting all locals into one kernel
 * triggers an alignment bug on ARM. Once this is fixed (in llvm) remove this
 * kernel and uncomment the relevant lines in other_kernel.
 */
uchar4 __attribute__((kernel)) long2_only_kernel(uchar4 in)
{
    long2 long2_local = (long2){-4444, 5555};

    return long2_local.x;
}

uchar4 __attribute__((kernel)) other_kernel(uchar4 in)
{
    char char_local = 12;
    uchar uchar_local = 234;
    short short_local = -321;
    ushort ushort_local = 432;
    int int_local = 1234;
    uint uint_local = 2345;
    float float_local = 4.5f;
    long long_local = -77777;
    ulong ulong_local = 8888;
    double double_local = -456.5f;

    char2 char2_local = (char2){11, -22};
    uchar2 uchar2_local = (uchar2){33, 44};
    short2 short2_local = (short2){-555, 666};
    ushort2 ushort2_local = (ushort2){777, 888};
    int2 int2_local = (int2){999, -1111};
    uint2 uint2_local = (uint2){2222, 3333};
    float2 float2_local = (float2){4.5f, -5.0f};
    // long2 long2_local = (long2){-4444, 5555};
    ulong2 ulong2_local = (ulong2){6666, 7777};
    double2 double2_local = (double2){88.5f, -99.0f};

    char3 char3_local = (char3){11, -22, -33};
    uchar3 uchar3_local = (uchar3){33, 44, 55};
    short3 short3_local = (short3){-555, 666, 777};
    ushort3 ushort3_local = (ushort3){777, 888, 999};
    int3 int3_local = (int3){999, -1111, 2222};
    uint3 uint3_local = (uint3){2222, 3333, 4444};
    float3 float3_local = (float3){4.5f, -5.0f, -6.5f};
    long3 long3_local = (long3){-4444, 5555, 6666};
    ulong3 ulong3_local = (ulong3){6666, 7777, 8888};
    double3 double3_local = (double3){88.5f, -99.0f, 111.5f};

    char4 char4_local = (char4){55, 11, -22, -33};
    uchar4 uchar4_local = (uchar4){222, 33, 44, 55};
    short4 short4_local = (short4){-444, -555, 666, 777};
    ushort4 ushort4_local = (ushort4){666, 777, 888, 999};
    int4 int4_local = (int4){888, 999, -1111, 2222};
    uint4 uint4_local = (uint4){1111, 2222, 3333, 4444};
    float4 float4_local = (float4){3.0f, 4.5f, -5.0f, -6.5f};
    long4 long4_local = (long4){-3333, -4444, 5555, 6666};
    ulong4 ulong4_local = (ulong4){5555, 6666, 7777, 8888};
    double4 double4_local = (double4){-77.0f, 88.5f, -99.0f, 111.5f};

    return char_local + uchar_local + short_local + ushort_local + int_local +
        uint_local + float_local + long_local + ulong_local + double_local +
        char2_local.x + uchar2_local.x + short2_local.x + ushort2_local.x +
        int2_local.x + uint2_local.x + float2_local.x + // long2_local.x +
        ulong2_local.x + double2_local.x + char3_local.x + uchar3_local.x +
        short3_local.x + ushort3_local.x + int3_local.x + uint3_local.x +
        float3_local.x + long3_local.x + ulong3_local.x + double3_local.x +
        char4_local.x + uchar4_local.x + short4_local.x + ushort4_local.x +
        int4_local.x + uint4_local.x + float4_local.x + long4_local.x +
        ulong4_local.x + double4_local.x;
}

