#pragma version(1)
#pragma rs java_package_name(com.android.rs.javafunction)
#pragma rs_fp_relaxed

float4 gColour = {0.299f, 0.587f, 0.114f, 1.f};

static float4 getColour()
{
    return gColour;
}

uchar4 __attribute__((kernel)) simple_kernel(uchar4 in)
{  
    float4 temp = getColour();
    return rsPackColorTo8888(temp);
}
