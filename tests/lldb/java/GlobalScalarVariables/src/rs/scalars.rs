#pragma version(1)
#pragma rs java_package_name(com.android.rs.globalscalarvariables)

char   c = 0x10;
short  s = 0x100;
int    i = 0x1000;
long   l = 0x10000;
float  f = 2.f;
double d = 2.0;

static void modify_i(int * a)
{
    int tmp = 10;
    *a = tmp;
}

static void modify_f(float * f)
{
    *f *= 0.5f;
}

int __attribute__((kernel)) simple_kernel(int in)
{
    float f_ = f; int i_ = i;
    modify_f(&f_);
    modify_i(&i_);
    int ret = (int) f;
    return in * ret;
}

