#pragma version(1)
#pragma rs java_package_name(com.android.rs.test)
#pragma rs_fp_relaxed

int4 __attribute__((kernel)) increment(int4 in)
{
	return in + 1;
}
