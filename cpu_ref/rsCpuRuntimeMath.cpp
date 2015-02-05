/*
 * Copyright (C) 2011-2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if !defined(RS_SERVER) && !defined(RS_COMPATIBILITY_LIB)
#include <cutils/compiler.h>
#endif

#include "rsContext.h"
#include "rsScriptC.h"
#include "rsMatrix4x4.h"
#include "rsMatrix3x3.h"
#include "rsMatrix2x2.h"

#include "rsCpuCore.h"
#include "rsCpuScript.h"

using namespace android;
using namespace android::renderscript;

typedef unsigned short half_t;

float gnu_h2f_ieee(short param) {
    unsigned short expHalf16 = param & 0x7C00;
    int exp1 = (int)expHalf16;
    unsigned short mantissa16 = param & 0x03FF;
    int mantissa1 = (int)mantissa16;
    int sign = (int)(param & 0x8000);
    sign = sign << 16;

    // nan or inf
    if (expHalf16 == 0x7C00) {
        // nan
        if (mantissa16 > 0) {
            int res = (0x7FC00000 | sign);
            float fres = *((float*)(&res));
            return fres;
        }
        // inf
        int res = (0x7F800000 | sign);
        float fres = *((float*)(&res));
        return fres;
    }
    if (expHalf16 != 0) {
        exp1 += ((127 - 15) << 10); //exponents converted to float32 bias
        int res = (exp1 | mantissa1);
        res = res << 13 ;
        res = ( res | sign );
        float fres = *((float*)(&res));
        return fres;
    }

    int xmm1 = exp1 > (1 << 10) ? exp1 : (1 << 10);
    xmm1 = (xmm1 << 13);
    xmm1 += ((127 - 15 - 10) << 23);  // add the bias difference to xmm1
    xmm1 = xmm1 | sign; // Combine with the sign mask

    float res = (float)mantissa1;  // Convert mantissa to float
    res *= *((float*) (&xmm1));

    return res;
}

short gnu_f2h_ieee(float param) {
    unsigned int param_bit = *((unsigned int*)(&param));
    int sign = param_bit >> 31;
    int mantissa = param_bit & 0x007FFFFF;
    int exp = ((param_bit & 0x7F800000) >> 23) + 15 - 127;
    short res;
    if (exp > 0 && exp < 30) {
        // use rte rounding mode, round the significand, combine sign, exponent and significand into a short.
        res = (sign << 15) | (exp << 10) | ((mantissa + 0x00001000) >> 13);
    } else if (param_bit == 0) {
        res = 0;
    } else  {
        if (exp <= 0) {
            if (exp < -10) {
                // value is less than min half float point
                res = 0;
            }  else {
                // normalized single, magnitude is less than min normal half float point.
                mantissa = (mantissa | 0x00800000) >> (1 - exp);
                // round to nearest
                if ((mantissa & 0x00001000) > 0) {
                    mantissa = mantissa + 0x00002000;
                }
                // combine sign & mantissa (exp is zero to get denormalized number)
                res = (sign << 15) | (mantissa >> 13);
            }
        } else if (exp == (255 - 127 + 15)) {
            if (mantissa == 0) {
                // input float is infinity, return infinity half
                res = (sign << 15) | 0x7C00;
            } else {
                // input float is NaN, return half NaN
                res = (sign << 15) | 0x7C00 | (mantissa >> 13);
            }
        } else {
            // exp > 0, normalized single, round to nearest
            if ((mantissa & 0x00001000) > 0) {
                mantissa = mantissa + 0x00002000;
                if ((mantissa & 0x00800000) > 0) {
                    mantissa = 0;
                    exp = exp + 1;
                }
            }
            if (exp > 30) {
                // exponent overflow - return infinity half
                res = (sign << 15) | 0x7C00;
            } else {
                // combine sign, exp and mantissa into normalized half
                res = (sign << 15) | (exp << 10) | (mantissa >> 13);
            }
        }
    }
    return res;
}

#define EXPORT_F32_FN_F32(func)                                 \
    float __attribute__((overloadable)) SC_##func(float v) {    \
        return func(v);                                         \
    }

#define EXPORT_F32_FN_F32_F32(func)                                     \
    float __attribute__((overloadable)) SC_##func(float t, float v) {   \
        return func(t, v);                                              \
    }

#define EXPORT_F16_FN_F16(func)                                   \
    half_t __attribute__((overloadable)) SC_##func##h(half_t v) { \
        float v1 = gnu_h2f_ieee(v);                               \
        return gnu_f2h_ieee(func##f(v1));                         \
    }

#define EXPORT_F16_FN_F16_F16(func)                                         \
    half_t __attribute__((overloadable)) SC_##func##h(half_t t, half_t v) { \
        float t1 = gnu_h2f_ieee(t);                                         \
        float v1 = gnu_h2f_ieee(v);                                         \
        return gnu_f2h_ieee(func##f(t1, v1));                               \
    }

//////////////////////////////////////////////////////////////////////////////
// Float util
//////////////////////////////////////////////////////////////////////////////

// Handle missing Gingerbread functions like tgammaf.
float SC_tgammaf(float x) {
#ifdef RS_COMPATIBILITY_LIB
    return tgamma(x);
#else
    return tgammaf(x);
#endif
}

half_t SC_tgammah(half_t x) {
#ifdef RS_COMPATIBILITY_LIB
    return gnu_f2h_ieee(tgamma(gnu_h2f_ieee(x)));
#else
    return gnu_f2h_ieee(tgammaf(gnu_h2f_ieee(x)));
#endif
}

uint32_t SC_abs_i32(int32_t v) {return abs(v);}

static void SC_MatrixLoadRotate(Matrix4x4 *m, float rot, float x, float y, float z) {
    m->loadRotate(rot, x, y, z);
}
static void SC_MatrixLoadScale(Matrix4x4 *m, float x, float y, float z) {
    m->loadScale(x, y, z);
}
static void SC_MatrixLoadTranslate(Matrix4x4 *m, float x, float y, float z) {
    m->loadTranslate(x, y, z);
}
static void SC_MatrixRotate(Matrix4x4 *m, float rot, float x, float y, float z) {
    m->rotate(rot, x, y, z);
}
static void SC_MatrixScale(Matrix4x4 *m, float x, float y, float z) {
    m->scale(x, y, z);
}
static void SC_MatrixTranslate(Matrix4x4 *m, float x, float y, float z) {
    m->translate(x, y, z);
}

static void SC_MatrixLoadOrtho(Matrix4x4 *m, float l, float r, float b, float t, float n, float f) {
    m->loadOrtho(l, r, b, t, n, f);
}
static void SC_MatrixLoadFrustum(Matrix4x4 *m, float l, float r, float b, float t, float n, float f) {
    m->loadFrustum(l, r, b, t, n, f);
}
static void SC_MatrixLoadPerspective(Matrix4x4 *m, float fovy, float aspect, float near, float far) {
    m->loadPerspective(fovy, aspect, near, far);
}

static bool SC_MatrixInverse_4x4(Matrix4x4 *m) {
    return m->inverse();
}
static bool SC_MatrixInverseTranspose_4x4(Matrix4x4 *m) {
    return m->inverseTranspose();
}
static void SC_MatrixTranspose_4x4(Matrix4x4 *m) {
    m->transpose();
}
static void SC_MatrixTranspose_3x3(Matrix3x3 *m) {
    m->transpose();
}
static void SC_MatrixTranspose_2x2(Matrix2x2 *m) {
    m->transpose();
}

float SC_randf2(float min, float max) {
    float r = (float)rand();
    r /= RAND_MAX;
    r = r * (max - min) + min;
    return r;
}

half_t SC_randh2(half_t min, half_t max) {
  return gnu_f2h_ieee(SC_randf2(gnu_h2f_ieee(min), gnu_h2f_ieee(max)));
}

static float SC_frac(float v) {
    int i = (int)floor(v);
    return fmin(v - i, 0x1.fffffep-1f);
}

EXPORT_F32_FN_F32(acosf)
EXPORT_F32_FN_F32(acoshf)
EXPORT_F32_FN_F32(asinf)
EXPORT_F32_FN_F32(asinhf)
EXPORT_F32_FN_F32(atanf)
EXPORT_F32_FN_F32_F32(atan2f)
EXPORT_F32_FN_F32(atanhf)
EXPORT_F32_FN_F32(cbrtf)
EXPORT_F32_FN_F32(ceilf)
EXPORT_F32_FN_F32_F32(copysignf)
EXPORT_F32_FN_F32(cosf)
EXPORT_F32_FN_F32(coshf)
EXPORT_F32_FN_F32(erfcf)
EXPORT_F32_FN_F32(erff)
EXPORT_F32_FN_F32(expf)
EXPORT_F32_FN_F32(exp2f)
EXPORT_F32_FN_F32(expm1f)
EXPORT_F32_FN_F32_F32(fdimf)
EXPORT_F32_FN_F32(floorf)
EXPORT_F32_FN_F32_F32(fmaxf)
EXPORT_F32_FN_F32_F32(fminf)
EXPORT_F32_FN_F32_F32(fmodf)
EXPORT_F32_FN_F32_F32(hypotf)
EXPORT_F32_FN_F32(lgammaf)
EXPORT_F32_FN_F32(logf)
EXPORT_F32_FN_F32(log10f)
EXPORT_F32_FN_F32(log1pf)
EXPORT_F32_FN_F32(logbf)
EXPORT_F32_FN_F32_F32(nextafterf)
EXPORT_F32_FN_F32_F32(powf)
EXPORT_F32_FN_F32_F32(remainderf)
EXPORT_F32_FN_F32(rintf)
EXPORT_F32_FN_F32(roundf)
EXPORT_F32_FN_F32(sinf)
EXPORT_F32_FN_F32(sinhf)
EXPORT_F32_FN_F32(sqrtf)
EXPORT_F32_FN_F32(tanf)
EXPORT_F32_FN_F32(tanhf)
EXPORT_F32_FN_F32(truncf)
float SC_fmaf(float u, float t, float v) {return fmaf(u, t, v);}
float SC_frexpf(float v, int* ptr) {return frexpf(v, ptr);}
int SC_ilogbf(float v) {return ilogbf(v); }
float SC_ldexpf(float v, int i) {return ldexpf(v, i);}
float SC_lgammaf_r(float v, int* ptr) {return lgammaf_r(v, ptr);}
float SC_modff(float v, float* ptr) {return modff(v, ptr);}
float SC_remquof(float t, float v, int* ptr) {return remquof(t, v, ptr);}


EXPORT_F16_FN_F16(acos)
EXPORT_F16_FN_F16(acosh)
EXPORT_F16_FN_F16(asin)
EXPORT_F16_FN_F16(asinh)
EXPORT_F16_FN_F16(atan)
EXPORT_F16_FN_F16_F16(atan2)
EXPORT_F16_FN_F16(atanh)
EXPORT_F16_FN_F16(cbrt)
EXPORT_F16_FN_F16(ceil)
EXPORT_F16_FN_F16_F16(copysign)
EXPORT_F16_FN_F16(cos)
EXPORT_F16_FN_F16(cosh)
EXPORT_F16_FN_F16(erfc)
EXPORT_F16_FN_F16(erf)
EXPORT_F16_FN_F16(exp)
EXPORT_F16_FN_F16(exp2)
EXPORT_F16_FN_F16(expm1)
EXPORT_F16_FN_F16_F16(fdim)
EXPORT_F16_FN_F16(floor)
EXPORT_F16_FN_F16_F16(fmax)
EXPORT_F16_FN_F16_F16(fmin)
EXPORT_F16_FN_F16_F16(fmod)
EXPORT_F16_FN_F16_F16(hypot)
EXPORT_F16_FN_F16(lgamma)
EXPORT_F16_FN_F16(log)
EXPORT_F16_FN_F16(log10)
EXPORT_F16_FN_F16(log1p)
EXPORT_F16_FN_F16(logb)
EXPORT_F16_FN_F16_F16(nextafter)
EXPORT_F16_FN_F16_F16(pow)
EXPORT_F16_FN_F16_F16(remainder)
EXPORT_F16_FN_F16(rint)
EXPORT_F16_FN_F16(round)
EXPORT_F16_FN_F16(sin)
EXPORT_F16_FN_F16(sinh)
EXPORT_F16_FN_F16(sqrt)
EXPORT_F16_FN_F16(tan)
EXPORT_F16_FN_F16(tanh)
EXPORT_F16_FN_F16(trunc)
half_t SC_fmah(half_t u, half_t t, half_t v) {return gnu_f2h_ieee(fmaf(gnu_h2f_ieee(u), gnu_h2f_ieee(t), gnu_h2f_ieee(v)));}
half_t SC_frexph(half_t v, int* ptr) {return gnu_f2h_ieee(frexpf(gnu_h2f_ieee(v), ptr));}
int SC_ilogbh(half_t v) {return gnu_f2h_ieee(ilogbf(gnu_h2f_ieee(v)));}
half_t SC_ldexph(half_t v, int i) {return gnu_f2h_ieee(ldexpf(gnu_h2f_ieee(v), i));}
half_t SC_lgammah_r(half_t v, int* ptr) {return gnu_f2h_ieee(lgammaf_r(gnu_h2f_ieee(v), ptr));}
half_t SC_modfh(half_t v, float* ptr) {return gnu_f2h_ieee(modff(gnu_h2f_ieee(v), ptr));}
half_t SC_remquoh(half_t t, half_t v, int* ptr) {return gnu_f2h_ieee(remquof(gnu_h2f_ieee(t), gnu_h2f_ieee(v), ptr));}
////////////////////////////////////////////////////////////////////////////////

float __attribute__((overloadable)) rsFrac(float f) {
    return SC_frac(f);
}
void __attribute__((overloadable)) rsMatrixLoadRotate(rs_matrix4x4 *m,
        float rot, float x, float y, float z) {
    SC_MatrixLoadRotate((Matrix4x4 *) m, rot, x, y, z);
}
void __attribute__((overloadable)) rsMatrixLoadScale(rs_matrix4x4 *m,
        float x, float y, float z) {
    SC_MatrixLoadScale((Matrix4x4 *) m, x, y, z);
}
void __attribute__((overloadable)) rsMatrixLoadTranslate(rs_matrix4x4 *m,
        float x, float y, float z) {
    SC_MatrixLoadTranslate((Matrix4x4 *) m, x, y, z);
}
void __attribute__((overloadable)) rsMatrixRotate(rs_matrix4x4 *m, float rot,
        float x, float y, float z) {
    SC_MatrixRotate((Matrix4x4 *) m, rot, x, y, z);
}
void __attribute__((overloadable)) rsMatrixScale(rs_matrix4x4 *m, float x,
        float y, float z) {
    SC_MatrixScale((Matrix4x4 *) m, x, y, z);
}
void __attribute__((overloadable)) rsMatrixTranslate(rs_matrix4x4 *m, float x,
        float y, float z) {
    SC_MatrixTranslate((Matrix4x4 *) m, x, y, z);
}
void __attribute__((overloadable)) rsMatrixLoadOrtho(rs_matrix4x4 *m, float l,
        float r, float b, float t, float n, float f) {
    SC_MatrixLoadOrtho((Matrix4x4 *) m, l, r, b, t, n, f);
}
void __attribute__((overloadable)) rsMatrixLoadFrustum(rs_matrix4x4 *m,
        float l, float r, float b, float t, float n, float f) {
    SC_MatrixLoadFrustum((Matrix4x4 *) m, l, r, b, t, n, f);
}
void __attribute__((overloadable)) rsMatrixLoadPerspective(rs_matrix4x4 *m,
        float fovy, float aspect, float near, float far) {
    SC_MatrixLoadPerspective((Matrix4x4 *) m, fovy, aspect, near, far);
}
bool __attribute__((overloadable)) rsMatrixInverse(rs_matrix4x4 *m) {
    return SC_MatrixInverse_4x4((Matrix4x4 *) m);
}
bool __attribute__((overloadable)) rsMatrixInverseTranspose(rs_matrix4x4 *m) {
    return SC_MatrixInverseTranspose_4x4((Matrix4x4 *) m);
}
void __attribute__((overloadable)) rsMatrixTranspose(rs_matrix4x4 *m) {
    SC_MatrixTranspose_4x4((Matrix4x4 *) m);
}
void __attribute__((overloadable)) rsMatrixTranspose(rs_matrix3x3 *m) {
    SC_MatrixTranspose_3x3((Matrix3x3 *) m);
}
void __attribute__((overloadable)) rsMatrixTranspose(rs_matrix2x2 *m) {
    SC_MatrixTranspose_2x2((Matrix2x2 *) m);
}

//////////////////////////////////////////////////////////////////////////////
// Class implementation
//////////////////////////////////////////////////////////////////////////////

// llvm name mangling ref
//  <builtin-type> ::= v  # void
//                 ::= b  # bool
//                 ::= c  # char
//                 ::= a  # signed char
//                 ::= h  # unsigned char
//                 ::= s  # short
//                 ::= t  # unsigned short
//                 ::= i  # int
//                 ::= j  # unsigned int
//                 ::= l  # long
//                 ::= m  # unsigned long
//                 ::= x  # long long, __int64
//                 ::= y  # unsigned long long, __int64
//                 ::= f  # float
//                 ::= d  # double

static RsdCpuReference::CpuSymbol gSyms[] = {
    { "_Z4acosf", (void *)&acosf, true },
    { "_Z5acoshf", (void *)&acoshf, true },
    { "_Z4asinf", (void *)&asinf, true },
    { "_Z5asinhf", (void *)&asinhf, true },
    { "_Z4atanf", (void *)&atanf, true },
    { "_Z5atan2ff", (void *)&atan2f, true },
    { "_Z5atanhf", (void *)&atanhf, true },
    { "_Z4cbrtf", (void *)&cbrtf, true },
    { "_Z4ceilf", (void *)&ceilf, true },
    { "_Z8copysignff", (void *)&copysignf, true },
    { "_Z3cosf", (void *)&cosf, true },
    { "_Z4coshf", (void *)&coshf, true },
    { "_Z4erfcf", (void *)&erfcf, true },
    { "_Z3erff", (void *)&erff, true },
    { "_Z3expf", (void *)&expf, true },
    { "_Z4exp2f", (void *)&exp2f, true },
    { "exp2f", (void *)&exp2f, true },
    { "_Z5expm1f", (void *)&expm1f, true },
    { "_Z4fdimff", (void *)&fdimf, true },
    { "_Z5floorf", (void *)&floorf, true },
    { "_Z3fmafff", (void *)&fmaf, true },
    { "_Z4fmaxff", (void *)&fmaxf, true },
    { "_Z4fminff", (void *)&fminf, true },
    { "_Z4fmodff", (void *)&fmodf, true },
    { "_Z5frexpfPi", (void *)&frexpf, true },
    { "_Z5hypotff", (void *)&hypotf, true },
    { "_Z5ilogbf", (void *)&ilogbf, true },
    { "_Z5ldexpfi", (void *)&ldexpf, true },
    { "_Z6lgammaf", (void *)&lgammaf, true },
    { "_Z6lgammafPi", (void *)&lgammaf_r, true },
    { "_Z3logf", (void *)&logf, true },
    { "_Z5log10f", (void *)&log10f, true },
    { "_Z5log1pf", (void *)&log1pf, true },
    { "_Z4logbf", (void *)&logbf, true },
    { "_Z4modffPf", (void *)&modff, true },
    { "_Z9nextafterff", (void *)&nextafterf, true },
    { "_Z3powff", (void *)&powf, true },
    { "powf", (void *)&powf, true },
    { "_Z9remainderff", (void *)&remainderf, true },
    { "_Z6remquoffPi", (void *)&remquof, true },
    { "_Z4rintf", (void *)&rintf, true },
    { "_Z5roundf", (void *)&roundf, true },
    { "_Z3sinf", (void *)&sinf, true },
    { "_Z4sinhf", (void *)&sinhf, true },
    { "_Z4sqrtf", (void *)&sqrtf, true },
    { "_Z3tanf", (void *)&tanf, true },
    { "_Z4tanhf", (void *)&tanhf, true },
    { "_Z6tgammaf", (void *)&SC_tgammaf, true },
    { "_Z5truncf", (void *)&truncf, true },

    // TODO add other mapping for half float
    { "_Z4acosDh", (void *)&SC_acosh, true },
    { "_Z5acoshDh", (void *)&SC_acoshh, true },
    { "_Z4asinDh", (void *)&SC_asinh, true },
    { "_Z5asinhDh", (void *)&SC_asinhh, true },
    { "_Z4atanDh", (void *)&SC_atanh, true },
    { "_Z5atan2fDh", (void *)&SC_atan2h, true },
    { "_Z5atanhDh", (void *)&SC_atanhh, true },
    { "_Z4cbrtDh", (void *)&SC_cbrth, true },
    { "_Z4ceilDh", (void *)&SC_ceilh, true },
    { "_Z8copysignfDh", (void *)&SC_copysignh, true },
    { "_Z3cosDh", (void *)&SC_cosh, true },
    { "_Z4coshDh", (void *)&SC_coshh, true },
    { "_Z4erfcDh", (void *)&SC_erfch, true },
    { "_Z3erfDh", (void *)&SC_erfh, true },
    { "_Z3expDh", (void *)&SC_exph, true },
    { "_Z4exp2Dh", (void *)&SC_exp2h, true },
    { "_Z5expm1Dh", (void *)&SC_expm1h, true },
    { "_Z4fdimfDh", (void *)&SC_fdimh, true },
    { "_Z5floorDh", (void *)&SC_floorh, true },
    { "_Z3fmaffDh", (void *)&SC_fmah, true },
    { "_Z4fmaxfDh", (void *)&SC_fmaxh, true },
    { "_Z4fminfDh", (void *)&SC_fminh, true },
    { "_Z4fmodfDh", (void *)&SC_fmodh, true },
    { "_Z5frexpfPi", (void *)&SC_frexph, true },
    { "_Z5hypotfDh", (void *)&SC_hypoth, true },
    { "_Z5ilogbDh", (void *)&SC_ilogbh, true },
    { "_Z5ldexpDhi", (void *)&SC_ldexph, true },
    { "_Z6lgammaDh", (void *)&SC_lgammah, true },
    { "_Z6lgammaDhPi", (void *)&SC_lgammaf_r, true },
    { "_Z3logDh", (void *)&SC_logh, true },
    { "_Z5log10Dh", (void *)&SC_log10h, true },
    { "_Z5log1pDh", (void *)&SC_log1ph, true },
    { "_Z4logbDh", (void *)&SC_logbh, true },
    { "_Z4modffPDh", (void *)&SC_modfh, true },
    { "_Z9nextafterfDh", (void *)&SC_nextafterh, true },
    { "_Z3powfDh", (void *)&SC_powh, true },
    { "_Z9remainderfDh", (void *)&SC_remainderh, true },
    { "_Z6remquofDhPi", (void *)&SC_remquoh, true },
    { "_Z4rintDh", (void *)&SC_rinth, true },
    { "_Z5roundDh", (void *)&SC_roundh, true },
    { "_Z3sinDh", (void *)&SC_sinh, true },
    { "_Z4sinhDh", (void *)&SC_sinhh, true },
    { "_Z4sqrtDh", (void *)&SC_sqrth, true },
    { "_Z3tanDh", (void *)&SC_tanh, true },
    { "_Z4tanhDh", (void *)&SC_tanhh, true },
    { "_Z6tgammaDh", (void *)&SC_tgammah, true },
    { "_Z5truncDh", (void *)&SC_trunch, true },

    // matrix
    { "_Z18rsMatrixLoadRotateP12rs_matrix4x4ffff", (void *)&SC_MatrixLoadRotate, true },
    { "_Z17rsMatrixLoadScaleP12rs_matrix4x4fff", (void *)&SC_MatrixLoadScale, true },
    { "_Z21rsMatrixLoadTranslateP12rs_matrix4x4fff", (void *)&SC_MatrixLoadTranslate, true },
    { "_Z14rsMatrixRotateP12rs_matrix4x4ffff", (void *)&SC_MatrixRotate, true },
    { "_Z13rsMatrixScaleP12rs_matrix4x4fff", (void *)&SC_MatrixScale, true },
    { "_Z17rsMatrixTranslateP12rs_matrix4x4fff", (void *)&SC_MatrixTranslate, true },

    { "_Z17rsMatrixLoadOrthoP12rs_matrix4x4ffffff", (void *)&SC_MatrixLoadOrtho, true },
    { "_Z19rsMatrixLoadFrustumP12rs_matrix4x4ffffff", (void *)&SC_MatrixLoadFrustum, true },
    { "_Z23rsMatrixLoadPerspectiveP12rs_matrix4x4ffff", (void *)&SC_MatrixLoadPerspective, true },

    { "_Z15rsMatrixInverseP12rs_matrix4x4", (void *)&SC_MatrixInverse_4x4, true },
    { "_Z24rsMatrixInverseTransposeP12rs_matrix4x4", (void *)&SC_MatrixInverseTranspose_4x4, true },
    { "_Z17rsMatrixTransposeP12rs_matrix4x4", (void *)&SC_MatrixTranspose_4x4, true },
    { "_Z17rsMatrixTransposeP12rs_matrix3x3", (void *)&SC_MatrixTranspose_3x3, true },
    { "_Z17rsMatrixTransposeP12rs_matrix2x2", (void *)&SC_MatrixTranspose_2x2, true },

    // RS Math
    { "_Z6rsRandff", (void *)&SC_randf2, true },
    { "_Z6rsFracf", (void *)&SC_frac, true },

    { nullptr, nullptr, false }
};

const RsdCpuReference::CpuSymbol * RsdCpuScriptImpl::lookupSymbolMath(const char *sym) {
    const RsdCpuReference::CpuSymbol *syms = gSyms;

    while (syms->fnPtr) {
        if (!strcmp(syms->name, sym)) {
            return syms;
        }
        syms++;
    }
    return nullptr;
}

