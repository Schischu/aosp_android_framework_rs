#pragma version(1)
#pragma rs java_package_name(com.android.rslib)
#pragma rs_fp_relaxed

// Simple invertion of the image


///////////////////////////UTILS////////////////////////////////////////
/**
 * Integer based rgb to hsv conversion
 * It has advantages of no loss in pressison
 */
static short3 rgb2hsv(uchar4 rgb) {
    int iMin, iMax, chroma;
    int ABITS = 4;
    int HSCALE = 256;

    int k1 = 255 << ABITS;
    int k2 = HSCALE << ABITS;

    int ri = rgb.r;
    int gi = rgb.g;
    int bi = rgb.b;
    short rv, rs, rh;

    if (ri > gi) {
        iMax = max(ri, bi);
        iMin = min(gi, bi);
    } else {
        iMax = max(gi, bi);
        iMin = min(ri, bi);
    }

    chroma = iMax - iMin;
    // set value
    rv = (short) (iMax << ABITS);

    // set saturation
    if (rv == 0)
        rs = 0;
    else
        rs = (short) ((k1 * chroma) / iMax);

    // set hue
    if (rs == 0)
        rh = 0;
    else {
        if (ri == iMax) {
            rh = (short) ((k2 * (6 * chroma + gi - bi)) / (6 * chroma));
            if (rh >= k2) rh -= k2;
        } else if (gi == iMax)
            rh = (short) ((k2 * (2 * chroma + bi - ri)) / (6 * chroma));
        else // (bi == iMax )
            rh = (short) ((k2 * (4 * chroma + ri - gi)) / (6 * chroma));
    }
    short3 hsv = {rv, rs, rh};
    return hsv;
}

/**
 * Integer based hsv to rgb conversion
 */
static uchar4 hsv2rgb(short3 hsv) {
    int ABITS = 4;
    int HSCALE = 256;
    int m;
    int H, X, ih, is, iv;
    int k1 = 255 << ABITS;
    int k2 = HSCALE << ABITS;
    int k3 = 1 << (ABITS - 1);
    int rr = 0;
    int rg = 0;
    int rb = 0;
    short cv = hsv.x;
    short cs = hsv.y;
    short ch = hsv.z;

    // set chroma and min component value m
    //chroma = ( cv * cs )/k1;
    //m = cv - chroma;
    m = ((int) cv * (k1 - (int) cs)) / k1;

    // chroma  == 0 <-> cs == 0 --> m=cv
    if (cs == 0) {
        rb = (rg = (rr = (cv >> ABITS)));
    } else {
        ih = (int) ch;
        is = (int) cs;
        iv = (int) cv;

        H = (6 * ih) / k2;
        X = ((iv * is) / k2) * (k2 - abs(6 * ih - 2 * (H >> 1) * k2 - k2));

        // removing additional bits --> unit8
        X = ((X + iv * (k1 - is)) / k1 + k3) >> ABITS;
        m = m >> ABITS;

        // ( chroma + m ) --> cv ;
        cv = (short) (cv >> ABITS);
        switch (H) {
            case 0:
                rr = cv;
                rg = X;
                rb = m;
                break;
            case 1:
                rr = X;
                rg = cv;
                rb = m;
                break;
            case 2:
                rr = m;
                rg = cv;
                rb = X;
                break;
            case 3:
                rr = m;
                rg = X;
                rb = cv;
                break;
            case 4:
                rr = X;
                rg = m;
                rb = cv;
                break;
            case 5:
                rr = cv;
                rg = m;
                rb = X;
                break;
        }
    }
    uchar4 out;
    out.r = rr;
    out.g = rg;
    out.b = rb;
    out.a = 255;
    return out;
}

/**
 * small process a pixel by a color 3x3 matrix
 */

static uchar4 colorMatrixMult3x3(uchar4 in, rs_matrix3x3 *m) {
    float4 pixel = rsUnpackColor8888(in);
    pixel.xyz = rsMatrixMultiply(m, pixel.xyz);
    return rsPackColorTo8888(clamp(pixel, 0.f, 1.0f));
}

static uchar4 colorMatrixMult4x4(uchar4 in, rs_matrix4x4 *m) {
    float4 pixel = rsUnpackColor8888(in);
    pixel = rsMatrixMultiply(m, pixel);
    pixel.w = 1;
    return rsPackColorTo8888(clamp(pixel, 0.f, 1.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// invert //////////////////////////////////////

/**
 * Inverts the image. Black becomes white etc.
 */
uchar4 __attribute__((kernel)) invert(uchar4 in) {
    uchar4 out = in;
    out.r = 255 - in.r;
    out.g = 255 - in.g;
    out.b = 255 - in.b;
    return out;
}

///////////////////////////// exposure //////////////////////////////////////
float exposureStrength = 0;
static float exposureMult = 1;

/**
 * Brightnees the image
 */
void setupExposure() {
    exposureMult = exposureStrength + 1;
}

uchar4 __attribute__((kernel)) exposure(uchar4 in) {
    float4 pixel = rsUnpackColor8888(in);
    pixel.xyz = exposureMult * pixel.xyz;
    return rsPackColorTo8888(clamp(pixel, 0.f, 1.0f));
}
///////////////////////////// Contrast //////////////////////////////////////

static float contrastMult;
static float contrastAdd;
float contrastStrength = 0;

void setupContrast() {
    contrastMult = pow(2.f, contrastStrength * 2);
    contrastAdd = .5f - contrastMult * .5f;
}

/**
 * Adjust the constrast of the image
 */
uchar4 __attribute__((kernel)) contrast(uchar4 in) {
    float4 pixel = rsUnpackColor8888(in);
    pixel.xyz = contrastMult * pixel.xyz + contrastAdd;
    return rsPackColorTo8888(clamp(pixel, 0.f, 1.0f));
}
//////////////////////// lens distortion code ///////////////////////////////

float correction1 = 1.f;
float correction2 = 0.f;
float correction3 = -.04f;
rs_allocation distortionImage;

static float2 image_size;
static float paramA;
static float paramB;
static float paramC;
static float paramD;
static float2 center;
static int radius;
rs_sampler sampler;

void setupDistortion() {
    int width = rsAllocationGetDimX(distortionImage);
    int height = rsAllocationGetDimY(distortionImage);
    image_size.x = width;
    image_size.y = height;
    center.x = width / 2.0f;
    center.y = height / 2.0f;
    radius = max(center.x, center.y);
    radius = max(center.x, center.y);
    paramA = correction1;
    paramB = correction2;
    paramC = correction3;
    paramD = 1.0 - paramA - paramB - paramC;

}

/**
 * Provides lens distortion effect
 */
uchar4 __attribute__((kernel)) distortion(uchar4 in, uint32_t x, uint32_t y) {
    const float2 p = {x, y};

    float2 delta = (p - center) / radius;
    float dstR = length(delta);
    float srcR = (paramA * dstR * dstR * dstR + paramB * dstR * dstR + paramC * dstR + paramD) * dstR;
    float factor = fabs(dstR / srcR);
    float2 srcP = center + (delta * factor * radius);
    srcP /= image_size;
    float4 fout = rsSample(distortionImage, sampler, srcP);
    return rsPackColorTo8888(fout);

}

///////////////////////////// SHADOWS //////////////////////////////////////
static float shadowFilterMap[] = {
        -0.00591f, 0.0001f,
        1.16488f, 0.01668f,
        -0.18027f, -0.06791f,
        -0.12625f, 0.09001f,
        0.15065f, -0.03897f
};
static float poly[5];

static double fastevalPoly(double x) {

    double f = x;
    double sum = poly[0] + poly[1] * f;
    int i;
    for (i = 2; i < 5; i++) {
        f *= x;
        sum += poly[i] * f;
    }
    return sum;
}

float shadowsStrength = 0;

void setupShadows() {
    double s = (shadowsStrength >= 0) ? shadowsStrength : shadowsStrength / 5;

    for (int i = 0; i < 5; i++) {
        poly[i] = shadowFilterMap[i * 2] + s * shadowFilterMap[i * 2 + 1];
    }
}

/**
 * Brightnees or darkens the darker regons of the image
 */
uchar4 __attribute__((kernel)) shadows(uchar4 in, uint32_t x, uint32_t y) {

    short3 hsv = rgb2hsv(in);
    float v = (fastevalPoly(hsv.x / 4080.f) * 4080);
    if (v > 4080) v = 4080;
    hsv.x = (unsigned short) ((v > 0) ? v : 0);

    uchar4 out = hsv2rgb(hsv);
    return out;
}

//////////////////////// Grain ///////////////////////////////

float grainStrength = 0;
#define GTABH  137
#define GTABW 139
float2 sGradient[GTABH * GTABW];
static bool tableEmpty = true;

static void fillTable() {
    for (int y = 0; y < GTABH; y++) {
        int p = y * GTABW;
        for (int x = 0; x < GTABW; x++) {
            float2 r;
            r.x = rsRand(1.f);
            r.y = rsRand(1.f);
            r = normalize(r);
            sGradient[p + x] = r;
        }
    }
}

static float dotGridGradient(int ix, int iy, float x, float y) {
    float2 p = {x, y};
    int2 ip = {ix, iy};

    float2 d = p - convert_float2(ip);

    return dot(d, sGradient[(ip.y % GTABH) * GTABW + ip.x % GTABW]);
}

static float perlin(float x, float y) {

    int x0 = (int) x;
    int y0 = (int) y;
    int y1 = y0 + 1;
    int x1 = x0 + 1;


    float sx = fract(x);
    float sy = fract(y);


    float n0, n1, ix0, ix1, value;

    n0 = dotGridGradient(x0, y0, x, y);
    n1 = dotGridGradient(x1, y0, x, y);
    ix0 = mix(n0, n1, sx);
    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    ix1 = mix(n0, n1, sx);
    value = mix(ix0, ix1, sy);

    return value;
}

void setupGrain() {
    if (tableEmpty) {
        fillTable();
    }
    tableEmpty = false;
}

/**
 * Simulates a grain like effect
 */
uchar4 __attribute__((kernel)) grain(uchar4 in, uint32_t x, uint32_t y) {
    float4 pixel = rsUnpackColor8888(in);
    pixel.r -= fabs(perlin(59.5f + x * .33333333f, 61.5f + y * .33333333f) * grainStrength);
    pixel.g -= fabs(perlin(71.5f + x * .33333333f, 73.5f + y * .33333333f) * grainStrength);
    pixel.b -= fabs(perlin(43.5f + x * .33333333f, 47.5f + y * .33333333f) * grainStrength);
    return rsPackColorTo8888(clamp(pixel, 0.f, 1.0f));
}

////////////////////////  Saturation /////////////////////////////////////////////////////
/**
 * adjust the saturation of the image
 */
float saturationStrength = 0;

uchar4 __attribute__((kernel)) saturation(uchar4 in, uint32_t x, uint32_t y) {
    float Rf = 0.2999f;
    float Gf = 0.587f;
    float Bf = 0.114f;
    float S = saturationStrength;;
    float MS = 1.0f - S;
    float Rt = Rf * MS;
    float Gt = Gf * MS;
    float Bt = Bf * MS;
    float R, G, B;

    int t = (in.r + in.g) / 2;

    float Rc = in.r * (Rt + S) + in.g * Gt + in.b * Bt;
    float Gc = in.r * Rt + in.g * (Gt + S) + in.b * Bt;
    float Bc = in.r * Rt + in.g * Gt + in.b * (Bt + S);

    uchar4 out;
    out.r = Rc;
    out.g = Gc;
    out.b = Bc;

    return out;
}

//////////////////////// temperature  ////////////////////////////////
/**
 * temprature will adjust from and assumed temprature of 5000
 */

float temprature = 5000;

static float3 color(float kelvin) {
    float centiKelven = kelvin / 100;
    float3 color = {255, 255, 255};
    if (centiKelven > 66) {
        float tmp = centiKelven - 60.f;
        color.r = (329.698727446f * pow(tmp, -0.1332047592f));
        color.g = (288.1221695283f * pow(tmp, 0.0755148492f));

    } else {
        color.g = (99.4708025861 * log(centiKelven) - 161.1195681661);

    }
    if (centiKelven < 66) {
        if (centiKelven > 19) {
            color.b = (138.5177312231 * log(centiKelven - 10) - 305.0447927307);
        } else {
            color.b = 0;
        }
    }

    color = clamp(color, 0.f, 255.f);
    return color;
}

static float3 tempratureScale;

void setupTemperature() {
    tempratureScale = color(temprature);
    tempratureScale /= color(5000); // default temp;
}

/**
 * Adjust the color temprature of the image
 */
uchar4 __attribute__((kernel)) temperature(uchar4 in, uint32_t x, uint32_t y) {

    float4 pixel = rsUnpackColor8888(in);
    pixel.xyz = pixel.xyz * tempratureScale;
    return rsPackColorTo8888(clamp(pixel, 0.f, 1.0f));
}

//////////////////////// sharpen  ////////////////////////////////
/**
 * defines shapening 0 = no shpen , 1 = max shapen -1 = blur
 */
float sharpenStrength = 0;

void setupSharpen() {

}

/**
 * Sharpens the image
 */
uchar4 __attribute__((kernel)) sharpen(uchar4 in, uint32_t x, uint32_t y) {
    uchar4 out = in;
    // TODO
    return out;
}
//////////////////////// hue code ////////////////////////////////////////

static rs_matrix3x3 hueMatrix;
static rs_matrix4x4 hueMatrix2;
#define RLUM  0.3086f
#define GLUM  0.6094f
#define BLUM  0.0820f

/*
computed using Maxima:
rotx(rs,rc):= matrix([1,0,0,0],[0,rc,rs,0],[0,-rs,rc,0],[0,0,0,1]);
roty(rs,rc):=matrix([rc,0,-rs,0],[0,1,0,0],[rs,0,rc,0],[0,0,0,1]);
zshearmat(dx,dy):=matrix([1,0,dx,0],[0,1,dy,0],[0,0,1,0],[0,0,0,1]);
rotz(rs,rc):=matrix([rc,rs,0,0],[-rs,rc,0,0],[0,0,0,0],[0,0,0,1]);
xformpnt(m ,x,y,z):= m . matrix([x],[y],[z]);
RLUM : (0.3086);
GLUM : (0.6094);
BLUM : (0.0820);
mag:sqrt(2.0);
xrs:1.0/mag;
xrc:1.0/mag;
m : rotx(xrs,xrc);
mag : sqrt(3.0);
yrs : -1.0/mag;
yrc : sqrt(2.0)/mag;
m2:m . roty(yrs,yrc);
ml:m2.matrix([RLUM],[GLUM],[BLUM],[1]);
zsx :ml[1]/ml[3];
zsy : ml[2]/ml[3];
m3:m2.zshearmat(zsx,zsy);
zrs:sin(r);
zrc:cos(r);
m4:m3.rotz(zrs,zrc);
m5:m4.zshearmat(-zsx,-zsy);
m6:m5.roty(-yrs,yrc);
m7:m6.rotx(-xrs,xrc);
string(m7);
*/
float hueValue = 0; // range -180..0...180

static void multiply(float *a, float *matrix) {
    int x, y;
    float temp[16];

    for (y = 0; y < 4; y++) {
        int y4 = y * 4;
        for (x = 0; x < 4; x++) {
            temp[y4 + x] = matrix[y4 + 0] * a[x]
                    + matrix[y4 + 1] * a[4 + x]
                    + matrix[y4 + 2] * a[8 + x]
                    + matrix[y4 + 3] * a[12 + x];
        }
    }
    for (int i = 0; i < 16; i++)
        matrix[i] = temp[i];
}

static void xRotateMatrix(float rs, float rc, float *matrix) {
    float tmp[16];
    tmp[0] = 1.f;
    tmp[15] = 1.f;
    tmp[5] = rc;
    tmp[6] = rs;
    tmp[9] = -rs;
    tmp[10] = rc;

    multiply(tmp, matrix);
}

static void yRotateMatrix(float rs, float rc, float *matrix) {

    float tmp[16];

    tmp[5] = 1.f;
    tmp[15] = 1.f;

    tmp[0] = rc;
    tmp[2] = -rs;
    tmp[8] = rs;
    tmp[10] = rc;

    multiply(tmp, matrix);
}

static void zRotateMatrix(float rs, float rc, float *matrix) {

    float tmp[16];

    tmp[10] = 1.f;
    tmp[15] = 1.f;
    tmp[0] = rc;
    tmp[1] = rs;
    tmp[4] = -rs;
    tmp[5] = rc;
    multiply(tmp, matrix);
}

static void zShearMatrix(float dx, float dy, float *matrix) {
    float tmp[16];
    tmp[0] = 1.f;
    tmp[5] = 1.f;
    tmp[10] = 1.f;
    tmp[15] = 1.f;

    tmp[2] = dx;
    tmp[6] = dy;
    multiply(tmp, matrix);
}

static float getRedf(float r, float g, float b, float *matrix) {
    return r * matrix[0] + g * matrix[4] + b * matrix[8] + matrix[12];
}

static float getGreenf(float r, float g, float b, float *matrix) {
    return r * matrix[1] + g * matrix[5] + b * matrix[9] + matrix[13];
}

static float getBluef(float r, float g, float b, float *matrix) {
    return r * matrix[2] + g * matrix[6] + b * matrix[10] + matrix[14];
}

static void hueCalc(float rot, float *matrix) {
    for (int i = 0; i < 16; i++) {
        matrix[i] = 0;
    }
    matrix[0] = 1.f;
    matrix[5] = 1.f;
    matrix[10] = 1.f;
    matrix[15] = 1.f;
    float mag = sqrt(2.0f);
    float xrs = 1 / mag;
    float xrc = 1 / mag;
    xRotateMatrix(xrs, xrc, matrix);
    mag = sqrt(3.0f);
    float yrs = -1 / mag;
    float yrc = sqrt(2.0f) / mag;
    yRotateMatrix(yrs, yrc, matrix);

    float lx = getRedf(RLUM, GLUM, BLUM, matrix);
    float ly = getGreenf(RLUM, GLUM, BLUM, matrix);
    float lz = getBluef(RLUM, GLUM, BLUM, matrix);
    float zsx = lx / lz;
    float zsy = ly / lz;
    zShearMatrix(zsx, zsy, matrix);

    float zrs = sin(radians(rot));
    float zrc = cos(radians(rot));
    zRotateMatrix(zrs, zrc, matrix);
    zShearMatrix(-zsx, -zsy, matrix);
    yRotateMatrix(-yrs, yrc, matrix);
    xRotateMatrix(-xrs, xrc, matrix);
}


void setupHue() {
    float tmp[16];
    hueCalc(hueValue, tmp);

    for (int i = 0; i < 16; i++) {
        int x = i % 4;
        int y = i / 4;
        int p = y + x * 4;
        hueMatrix2.m[i] = tmp[p];
    }
    if (true) return;
    float cosr = cos(radians(hueValue));
    float sinr = sin(radians(hueValue));
    rsMatrixSet(&hueMatrix, 0, 0, 0.3258947002100784f * sinr + 0.9435713458210051f * cosr);
    rsMatrixSet(&hueMatrix, 1, 0, 0.9032449693997042f * sinr - 0.05642865417899519f * cosr);
    rsMatrixSet(&hueMatrix, 2, 0, -0.2514555689795475f * sinr - 0.05642865417899519f * cosr);

    rsMatrixSet(&hueMatrix, 0, 1, -0.9801041058691011f * sinr - 0.1895525835698608f * cosr);
    rsMatrixSet(&hueMatrix, 1, 1, 0.8104474164301392f * cosr - 0.4027538366794753f * sinr);
    rsMatrixSet(&hueMatrix, 2, 1, 0.1745964325101504f * sinr - 0.1895525835698606f * cosr);

    rsMatrixSet(&hueMatrix, 0, 2, 0.6542094056590227f * sinr - 0.7540187622511441f * cosr);
    rsMatrixSet(&hueMatrix, 1, 2, -0.5004911327202288f * sinr - 0.7540187622511438f * cosr);
    rsMatrixSet(&hueMatrix, 2, 2, 0.0768591364693969f * sinr + 0.2459812377488559f * cosr);
}


uchar4 __attribute__((kernel)) hue(uchar4 in, uint32_t x, uint32_t y) {
    //  return colorMatrixMult(in, &hueMatrix);
    return colorMatrixMult4x4(in, &hueMatrix2);

    // short3 hsv = rgb2hsv(in);
    //hsv.z = (hsv.z+(int)hueValue)%4080;
    //return hsv2rgb(hsv);

}

//////////////////////// look /////////////////////////////////

int LOOK;

void setupLook() {

}

uchar4 __attribute__((kernel)) look(uchar4 in, uint32_t x, uint32_t y) {
    uchar4 out = in;
    // lut
    return out;
}
//////////////////////// Vignette code /////////////////////////////////

float vignetteCenterX;
float vignetteCenterY;
float vignetteRadiusX;
float vignetteRadiusY;
float vignetteStrength;
float vignetteExposure;
float vignetteSaturation;
float vignetteContrast;
float vignetteAdd;

static rs_matrix3x3 vignetteColorMatrix;
static float vignetteScaleX;
static float vignetteScaleY;
static float vignetteOffset;
static const float Rf = 0.2999f;
static const float Gf = 0.587f;
static const float Bf = 0.114f;


void setupVignette() {
    int k = 0;

    vignetteScaleX = 1.f / vignetteRadiusX;
    vignetteScaleY = 1.f / vignetteRadiusY;

    float S = 1 + vignetteSaturation / 100.f;
    float MS = 1 - S;
    float Rt = Rf * MS;
    float Gt = Gf * MS;
    float Bt = Bf * MS;

    float b = 1 + vignetteExposure / 100.f;
    float c = 1 + vignetteContrast / 100.f;
    b *= c;
    vignetteOffset = .5f - c / 2.f + vignetteAdd / 100.f;
    rsMatrixSet(&vignetteColorMatrix, 0, 0, b * (Rt + S));
    rsMatrixSet(&vignetteColorMatrix, 1, 0, b * Gt);
    rsMatrixSet(&vignetteColorMatrix, 2, 0, b * Bt);
    rsMatrixSet(&vignetteColorMatrix, 0, 1, b * Rt);
    rsMatrixSet(&vignetteColorMatrix, 1, 1, b * (Gt + S));
    rsMatrixSet(&vignetteColorMatrix, 2, 1, b * Bt);
    rsMatrixSet(&vignetteColorMatrix, 0, 2, b * Rt);
    rsMatrixSet(&vignetteColorMatrix, 1, 2, b * Gt);
    rsMatrixSet(&vignetteColorMatrix, 2, 2, b * (Bt + S));
}

/*
 * adds a color change to the edges of an image
 */
uchar4 __attribute__((kernel)) vignette(const uchar4 in, uint32_t x, uint32_t y) {
    float4 pixel = rsUnpackColor8888(in);
    float radx = (x - vignetteCenterX) * vignetteScaleX;
    float rady = (y - vignetteCenterY) * vignetteScaleY;
    float dist = vignetteStrength * (sqrt(radx * radx + rady * rady) - 1.f);
    float t = (1.f + dist / sqrt(1.f + dist * dist)) * .5f;
    float4 wsum = pixel;
    wsum.xyz = wsum.xyz * (1 - t) + t * (rsMatrixMultiply(&vignetteColorMatrix, wsum.xyz) + vignetteOffset);
    wsum.a = 1.0f;
    uchar4 out = rsPackColorTo8888(clamp(wsum, 0.f, 1.0f));
    return out;
}

float dehazeStrength;
rs_allocation dehazeBigBlur;
rs_allocation dehazeSmallBlur;

/*
 * adds a color change to the edges of an image
 */
uchar4 __attribute__((kernel)) dehaze(const uchar4 in, uint32_t x, uint32_t y) {

    float4 pixel = rsUnpackColor8888(in);
    float4 b1 = rsUnpackColor8888(rsGetElementAt_uchar4(dehazeBigBlur, x, y));
    float4 b2 = rsUnpackColor8888(rsGetElementAt_uchar4(dehazeSmallBlur, x, y));
    float i = length(pixel.xyz);
    float blur1Fac = dehazeStrength * i * .2f;
    float blur2Fac = dehazeStrength * (1 - i) * .2f;

    float4 wsum = pixel - b1 * blur1Fac - b2 * blur2Fac - .2 * (1 - i);

    wsum.a = 1.0f;
    uchar4 out = rsPackColorTo8888(clamp(wsum, 0.f, 1.0f));
    return out;
}