#pragma version(1)
#pragma rs java_package_name(com.android.rs.closure)

rs_allocation a_in;
int reduction_stride;

int4 __attribute__((kernel)) add(uint x)
{
  return rsGetElementAt_int4(a_in, x) + rsGetElementAt_int4(a_in, x + reduction_stride);
}
