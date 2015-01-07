#pragma version(1)
#pragma rs java_package_name(com.android.rs.closure)

int4 __attribute__((kernel)) increment(int4 in)
{
  return in + 1;
}
