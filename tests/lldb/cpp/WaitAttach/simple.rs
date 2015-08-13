#pragma version(1)
#pragma rs java_package_name(com.codeplay.simple)

float4 gColor = {0.299f, 0.587f, 0.114f, 1.f};

uchar4 __attribute__((kernel)) simple_kernel(uchar4 in)
{
    float4 out = rsUnpackColor8888(in);

    out.r = gColor.r;
    out.g = gColor.g;
    out.b = gColor.b;
    out.a = gColor.a;

    uchar4 result = rsPackColorTo8888(out);
    return result;
}

