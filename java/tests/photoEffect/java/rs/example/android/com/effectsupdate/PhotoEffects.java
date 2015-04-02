package rs.example.android.com.effectsupdate;

import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.Sampler;
import android.renderscript.ScriptIntrinsic3DLUT;
import android.renderscript.ScriptIntrinsicBlur;
import android.renderscript.ScriptIntrinsicConvolve3x3;
import android.renderscript.ScriptIntrinsicConvolve5x5;
import android.renderscript.ScriptIntrinsicLUT;
import android.renderscript.Type;

import com.android.rslib.ScriptC_PhotoEffects;

/**
 * PhotoEffects filters provide basic photo filters
 */
public class PhotoEffects {
    ScriptC_PhotoEffects mPhotoEffects;
    ScriptIntrinsicConvolve5x5 sharpen;
    ScriptIntrinsicBlur blur;
    ScriptIntrinsic3DLUT lut;
    RenderScript mRs;
    Allocation lutAllocation;
    Allocation blurAllocation1;
    Allocation blurAllocation2;
    float[] mSharpenCoefficients;

    /**
     * Create an intrinsic for applying various photo processing effects to an allocation.
     * @param rs
     */
    PhotoEffects(RenderScript rs) {
        mPhotoEffects = new ScriptC_PhotoEffects(rs);
        mRs = rs;
    }

    /**
     * Invert the image. Black becomes white etc.
     *
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_invert(Allocation ain, Allocation aout) {
        mPhotoEffects.forEach_invert(ain, aout);
    }

    /**
     * Sets the exposure adjustment strength between -1 to 1.
     *
     * @param v 0 = no change -1 = very dark +1 = over exposeure
     */
    public synchronized void set_exposureStrength(float v) {
        mPhotoEffects.set_exposureStrength(v);
    }

    /**
     * Adjust the brightness of an image. Controlled by @set_exposureStrength
     *
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_exposure(Allocation ain, Allocation aout) {
        mPhotoEffects.invoke_setupExposure();
        mPhotoEffects.forEach_exposure(ain, aout);
    }

    /**
     * Sets the contrast adjustment strength between -1 to 1.
     *
     * @param v -0 = no change -1 = no contrast (all gray) +1 = very high contrast
     */
    public synchronized void set_contrastStrength(float v) {
        mPhotoEffects.set_contrastStrength(v);
    }

    /**
     * Adjust the contrast of an image. Controlled by @set_exposureStrength
     *
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_contrast(Allocation ain, Allocation aout) {
        mPhotoEffects.invoke_setupContrast();
        mPhotoEffects.forEach_contrast(ain, aout);
    }

    /**
     * parameters to controls the distortion
     *
     * @param v1 primary parameter affecting external part of image
     * @param v2 secondary parameter affecting the internal area
     * @param v3 third parameter corrects for wave distortions
     */
    public synchronized void set_correction(float v1, float v2, float v3) {
        mPhotoEffects.set_correction1(v1);
        mPhotoEffects.set_correction2(v2);
        mPhotoEffects.set_correction3(v3);
    }

    /**
     * Distort or correct the distortion of an image.
     *
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_distortion(Allocation ain, Allocation aout) {
        mPhotoEffects.set_distortionImage(ain);
        mPhotoEffects.set_sampler(Sampler.CLAMP_LINEAR(mRs));
        mPhotoEffects.invoke_setupDistortion();
        mPhotoEffects.forEach_distortion(ain, aout);
    }

    /**
     * Set the strength of the grain
     *
     * @param v grain strength 0 = no grain 1 = strong grain
     */
    public synchronized void set_grainStrength(float v) {
        mPhotoEffects.set_grainStrength(v);
    }

    /**
     * adds grain noise to an image
     *
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_grain(Allocation ain, Allocation aout) {
        mPhotoEffects.invoke_setupGrain();
        mPhotoEffects.forEach_grain(ain, aout);
    }

    /**
     * sets the saturation strength
     *
     * @param v -1 = grayscale 0 = no change +1 very saturated
     */
    public synchronized void set_saturationStrength(float v) {
        mPhotoEffects.set_saturationStrength(v);
    }

    /**
     * adjust the saturation of an image
     *
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_saturation(Allocation ain, Allocation aout) {
        mPhotoEffects.forEach_saturation(ain, aout);
    }

    /**
     * Adjust the color temperature of an image
     * Original image is assumed to be 5000 Kelven
     *
     * @param v 5000 = no change
     */
    public synchronized void set_temprature(float v) {
        mPhotoEffects.set_temprature(v);
    }

    /**
     * Adjust the color temperature of an image
     *
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_temperature(Allocation ain, Allocation aout) {
        mPhotoEffects.invoke_setupTemperature();
        mPhotoEffects.forEach_temperature(ain, aout);
    }

    /**
     * set the strength of the sharpen of an image
     *
     * @param v 0 = no change -1 = blur +1 = sharpen
     */
    public synchronized void set_sharpenStrength(float v) { // TODO Work in progress
        mPhotoEffects.set_sharpenStrength(v);

        float coeff1 = 2, coeff2 = -0.6931472f;

        float e = (float) Math.E;
        int w = 3;
        if (mSharpenCoefficients == null) {
            mSharpenCoefficients = new float[w * w];
        }
        float normalizeFactor = 0;


        for (int i = 0; i < w * w; i++) {
            int x = i % w - w / 2;
            int y = i / w - w / 2;
            mSharpenCoefficients[i] = coeff1 * (float) Math.exp((x * x + y * y) * coeff2);
            normalizeFactor += mSharpenCoefficients[i];
        }

    }

    /**
     * Sharpens image
     *
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_sharpen(Allocation ain, Allocation aout) {

        if (sharpen == null) {
            sharpen = ScriptIntrinsicConvolve5x5.create(mRs, Element.U8_4(mRs));
        }
        sharpen.setInput(ain);
        sharpen.setCoefficients(mSharpenCoefficients);
        mPhotoEffects.forEach_sharpen(ain, aout);
    }

    public synchronized void set_hueValue(float v) {  // TODO Work in progress
        mPhotoEffects.set_hueValue(v);
    }

    /**
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_hue(Allocation ain, Allocation aout) {
        mPhotoEffects.invoke_setupHue();
        mPhotoEffects.forEach_hue(ain, aout);
    }

    private static int cubeRoot(int len) {
        int i = (int) Math.pow(len, 1 / 3.);
        int error = i * i * i - len;
        if (error == 0) return i;
        i += (error > 0) ? -1 : 1;
        error = i * i * i - len;
        if (error == 0) return i;
        return 0;
    }

    /**
     * Set the 3D lookup table
     * @param lut
     */
    public synchronized void invoke_setupLook(int[] lut) {
        if (lutAllocation != null) {
            lutAllocation.destroy();
        }
        int w = cubeRoot(lut.length);
        if (w < 1) {
            throw new IllegalArgumentException("Look up table is not a cube");
        }
        if (w < 2) {
            throw new IllegalArgumentException("Look up table is too small");
        }
        Type.Builder b = new Type.Builder(mRs, Element.U8_4(mRs));
        b.setX(w);
        b.setY(w);
        b.setZ(w);

        lutAllocation = Allocation.createTyped(mRs, b.create());
        lutAllocation.copyFromUnchecked(lut);

    }

    /**
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_look(Allocation ain, Allocation aout) { // TODO work in progress
        if (lut == null) {
            lut = ScriptIntrinsic3DLUT.create(mRs, Element.U8_4(mRs));
        }

        lut.forEach(ain, aout);
    }

    public synchronized void set_shadowsStrength(float v) { // TODO Work in progress
        mPhotoEffects.set_shadowsStrength(v);
    }

    /**
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_shadows(Allocation ain, Allocation aout) {
        mPhotoEffects.invoke_setupShadows();
        mPhotoEffects.forEach_shadows(ain, aout);
    }

    /**
     * Sets the center location of the vignette
     *
     * @param x
     * @param y
     */
    public synchronized void set_vignetteCenter(float x, float y) {
        mPhotoEffects.set_vignetteCenterX(x);
        mPhotoEffects.set_vignetteCenterY(y);
    }

    /**
     * Sets the width and height of the ellipse
     *
     * @param x
     * @param y
     */
    public synchronized void set_vignetteRadius(float x, float y) {
        mPhotoEffects.set_vignetteRadiusX(x);
        mPhotoEffects.set_vignetteRadiusY(y);
    }

    /**
     * Set the saturation that occurs outside the elipse
     *
     * @param v -1 = grayscale 0 = no change +1 very saturated
     */
    public synchronized void set_vignetteSaturation(float v) {
        mPhotoEffects.set_vignetteSaturation(v);
    }

    /**
     * set the contrast outside the vignette ellipse
     *
     * @param v -0 = no change -1 = no contrast (all gray) +1 = very high contrast
     */
    public synchronized void set_vignetteContrast(float v) {
        mPhotoEffects.set_vignetteContrast(v);
    }

    /**
     * set the rate the vignette transitions from clear to full effect
     *
     * @param v 0 = no transitions 100 = instantaneous
     */
    public synchronized void set_vignetteRate(float v) {
        mPhotoEffects.set_vignetteStrength(v);
    }

    /**
     * set a value to subtract or add outside the vignette ellipse
     * two types of effects are available one where the Intensity scaled
     * another where a value is added to the Intensity.
     * fading to white generally looks better if you add
     *
     * @param v
     */
    public synchronized void set_vignetteAdd(float v) {
        mPhotoEffects.set_vignetteAdd(v);
    }

    /**
     * set a value to scale the outside the vignette ellipse
     * two types of effects are available one where the Intensity scaled
     * another where a value is added to the Intensity.
     * fading to white generally looks better if you add
     *
     * @param v 0 = no change -1 = very dark +1 = over exposeure
     */
    public synchronized void set_vignetteExposure(float v) {
        mPhotoEffects.set_vignetteExposure(v);
    }

    /**
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_vignette(Allocation ain, Allocation aout) {
        mPhotoEffects.invoke_setupVignette();
        mPhotoEffects.forEach_vignette(ain, aout);
    }


    public synchronized void set_dehazeStrength(float v) {// TODO Work in progress
        mPhotoEffects.set_dehazeStrength(v);
    }

    /**
     * @param ain  input image
     * @param aout output image
     */
    public void forEach_dehaze(Allocation ain, Allocation aout) {
        if (blur == null) {
            blur = ScriptIntrinsicBlur.create(mRs, Element.U8_4(mRs));
        }
        Type inType = ain.getType();
        if (blurAllocation1 != null) {
            Type scratchType = blurAllocation1.getType();

            if (scratchType.getX() != inType.getX() || scratchType.getY() != inType.getY()) {
                blurAllocation1.destroy();
                blurAllocation2.destroy();
            }
        }

        Type.Builder b = new Type.Builder(mRs, Element.U8_4(mRs));
        b.setX(inType.getX());
        b.setY(inType.getY());


        blurAllocation1 = Allocation.createTyped(mRs, b.create());
        blurAllocation2 = Allocation.createTyped(mRs, b.create());
        blur.setInput(ain);
        blur.setRadius(14);
        blur.forEach(blurAllocation1);
        mPhotoEffects.set_dehazeBigBlur(blurAllocation1);
        blur.setRadius(7);
        blur.forEach(blurAllocation2);
        mPhotoEffects.set_dehazeSmallBlur(blurAllocation2);

        mPhotoEffects.forEach_dehaze(ain, aout);
    }

    /**
     * Cleans up all resources used be this filter set
     */
    void destroy() {
        mPhotoEffects.destroy();
        if (lut != null) {
            lut.destroy();
        }
        if (sharpen != null) {
            sharpen.destroy();
        }
        if (blur != null) {
            blur.destroy();
        }

        if (blurAllocation1 != null) {
            blurAllocation1.destroy();
        }

        if (blurAllocation2 != null) {
            blurAllocation2.destroy();
        }

        if (lutAllocation != null) {
            lutAllocation.destroy();
        }
    }

    @Override
    protected void finalize() throws Throwable {
        destroy();
        super.finalize();
    }

    /**
     * A 3D lookup table give an vintage film look
     */
    public static final int[] LUT_VINTAGE = {
            0x001f36, 0x002036, 0x1c2038, 0x61263f, 0x9c2d46, 0xc4334d, 0xe13952, 0xf13d56,
            0x00213c, 0x00213a, 0x1a223c, 0x5e2642, 0x9a2d48, 0xc3344f, 0xe13a54, 0xf33e57,
            0x002244, 0x002343, 0x172342, 0x5e2747, 0x972e4e, 0xc23554, 0xe03b5a, 0xf7415f,
            0x00244d, 0x00254d, 0x23264c, 0x612a4e, 0x982f54, 0xc1355a, 0xde3b60, 0xf74165,
            0x002556, 0x052658, 0x302856, 0x672b57, 0x99305a, 0xc33760, 0xdf3b66, 0xf8426c,
            0x002760, 0x122862, 0x402c61, 0x6f2d60, 0xa03265, 0xc23867, 0xe13e6d, 0xf84373,
            0x20286c, 0x2a2b6d, 0x522f6e, 0x7a2f6c, 0xa2336e, 0xc73972, 0xe03f75, 0xf8457a,
            0x3a2c78, 0x392c78, 0x63307c, 0x8a327b, 0xac357d, 0xcb3b7e, 0xe3407f, 0xfb4783,
            0x00333b, 0x00373c, 0x39383f, 0x803846, 0xb3374e, 0xd33c53, 0xea3f57, 0xf84159,
            0x003541, 0x003843, 0x3a3943, 0x7f3a49, 0xb43951, 0xd43e55, 0xed4158, 0xfb445b,
            0x003649, 0x02394a, 0x353949, 0x78394d, 0xb03755, 0xd43e5b, 0xec425f, 0xfe4761,
            0x003752, 0x083953, 0x393a51, 0x753a53, 0xad395a, 0xd13d60, 0xea4365, 0xfe4769,
            0x00385a, 0x0f3a5b, 0x3e3a5a, 0x783b5c, 0xab3961, 0xd13e66, 0xea436b, 0xfd476f,
            0x0d3963, 0x1b3964, 0x4c3b64, 0x7c3c64, 0xac3c69, 0xcf406d, 0xe74371, 0xfe4a77,
            0x24396d, 0x2e3b6e, 0x583c70, 0x823c6f, 0xb13c73, 0xd04177, 0xe94379, 0xff4a7e,
            0x3a3977, 0x393a77, 0x653d7b, 0x8d3b7d, 0xb23e7e, 0xcf4180, 0xeb4485, 0x004b87,
            0x004a42, 0x004e43, 0x415647, 0x8b5a49, 0xb9584d, 0xdd5454, 0xf84e5d, 0xff5567,
            0x004a48, 0x004e49, 0x45584e, 0x8b5a50, 0xb95a51, 0xde5655, 0xfa505e, 0xff5567,
            0x004b4e, 0x065251, 0x4a5855, 0x8a5a56, 0xb85b58, 0xdc575a, 0xf75262, 0xff596d,
            0x004f58, 0x13535b, 0x4f575d, 0x885c5d, 0xb45b5f, 0xd95962, 0xf5536a, 0xff5e79,
            0x0e5061, 0x215363, 0x565665, 0x8e5e68, 0xb35e67, 0xd75a68, 0xf3556e, 0xff607f,
            0x27526c, 0x33536d, 0x61586f, 0x925c70, 0xb75f71, 0xd55d71, 0xef5d75, 0x016687,
            0x3b5176, 0x425476, 0x6a5779, 0x965a78, 0xb65c79, 0xd7607b, 0xed5c7c, 0x025e86,
            0x4f547f, 0x4c537f, 0x725782, 0x9b5c83, 0xbb5e84, 0xda6086, 0xee5587, 0x035e8c,
            0x00644a, 0x01684b, 0x296e4c, 0x8c784f, 0xc07c4e, 0xe27d51, 0xf87b58, 0x007861,
            0x006450, 0x046852, 0x316f53, 0x8b7958, 0xbf7c58, 0xe07f59, 0xf87f59, 0xff7c63,
            0x006657, 0x0d6a58, 0x40715b, 0x927a61, 0xbf7d61, 0xe08062, 0xf67e62, 0xff7e69,
            0x08675d, 0x156b5f, 0x547464, 0x987d6a, 0xc3806c, 0xdf806b, 0xf67d69, 0x008070,
            0x206a67, 0x306f6a, 0x65766f, 0x9c7d75, 0xbe7f73, 0xde8173, 0xf57e72, 0x008275,
            0x3d6e73, 0x467174, 0x73777a, 0xa27d7e, 0xc1807d, 0xdc827b, 0xf38079, 0x02837f,
            0x53707e, 0x54717f, 0x7b7783, 0xa67d88, 0xc68089, 0xde8487, 0xf28584, 0x028082,
            0x657388, 0x646f88, 0x85758e, 0xab7b92, 0xcb8696, 0xe08593, 0xf1828f, 0x02838a,
            0x0d7f52, 0x198253, 0x3a8955, 0x7a9459, 0xbd9a58, 0xe29d55, 0xfc9d50, 0xff9d5f,
            0x157e58, 0x20825a, 0x3f895c, 0x7e9360, 0xbe9c62, 0xe29e5f, 0xfa9e5c, 0xff9f62,
            0x1f7f5f, 0x278360, 0x418863, 0x839267, 0xbf9c6b, 0xe2a06a, 0xfba067, 0xffa26f,
            0x2c8366, 0x308467, 0x488a69, 0x8d936f, 0xc39f75, 0xe3a073, 0xfaa271, 0xffa57c,
            0x33856b, 0x37856c, 0x619074, 0x9b997a, 0xc99f7f, 0xe4a17b, 0xfaa37a, 0xffa789,
            0x498876, 0x498676, 0x77947f, 0xa19883, 0xcba08a, 0xe4a387, 0xfcaa88, 0xfeaf95,
            0x658c83, 0x608c81, 0x829289, 0xac9c90, 0xd2a698, 0xe7a798, 0xf8a690, 0xfeb4a0,
            0x799190, 0x778e8e, 0x8e9594, 0xb79f9e, 0xd4a4a1, 0xe7a6a2, 0xfcaca0, 0xffb2a8,
            0x2e9859, 0x349b5a, 0x4da05d, 0x72a85f, 0xb6b866, 0xdfb964, 0xf9bc5d, 0xffbc5e,
            0x33975f, 0x3b9a61, 0x54a265, 0x73a666, 0xb8b76f, 0xe3bc6d, 0xfcbf68, 0xfebc6b,
            0x3b9866, 0x3f9867, 0x569f6b, 0x74a66d, 0xbcb577, 0xe3bc77, 0xfbbe74, 0xffbf7c,
            0x469a6d, 0x4b9c6e, 0x5da171, 0x73a572, 0xbfb57e, 0xe7bf83, 0xfdc280, 0xffc289,
            0x4e9c73, 0x4e9a73, 0x64a277, 0x8eac7d, 0xc4b586, 0xeabf8d, 0xffc489, 0xffc392,
            0x559c78, 0x569d78, 0x66a37d, 0xa2b48a, 0xcaba8f, 0xedc297, 0x02c696, 0xffc59e,
            0x5f9c80, 0x629e82, 0x83aa8b, 0xadb694, 0xd2bd9d, 0xf1c3a4, 0x01c9a1, 0xffccac,
            0x81a792, 0x81a692, 0x93ae96, 0xb3b5a0, 0xd5bea7, 0xf2c4b0, 0x01c6ad, 0xffd0b4,
            0x46af59, 0x46af5a, 0x5db75f, 0x7ebe64, 0x97c167, 0xe3d776, 0xfbd470, 0xfed67c,
            0x4aae60, 0x49ad60, 0x61b666, 0x7ebc6b, 0x96c06d, 0xe2d57d, 0xfcd779, 0xffd886,
            0x50ad67, 0x4fad67, 0x65b56c, 0x82bc72, 0x98c076, 0xe0d386, 0xffd686, 0xffd893,
            0x55ac6d, 0x56ad6d, 0x68b373, 0x85bd79, 0x9bbf7d, 0xe2d18d, 0xffd991, 0xffdaa0,
            0x5aac73, 0x57a972, 0x6bb178, 0x86bb7d, 0xb7c78b, 0xe5d393, 0x02d99a, 0xffdbac,
            0x5fab78, 0x60aa79, 0x70b37d, 0x84b883, 0xc5cd96, 0xe6d39a, 0x03ddaa, 0xffe0b7,
            0x6aaf80, 0x68ad7f, 0x74b383, 0xa4c495, 0xcdd19f, 0xecd5a5, 0x01dab0, 0xffe1bd,
            0x6dad88, 0x6cab86, 0x91c097, 0xb0c8a0, 0xd5d2aa, 0xf1d8b1, 0x00dbbe, 0xffe4c7,
            0x58c05c, 0x57c05b, 0x6bc861, 0x85d065, 0xa3d86b, 0xc1e171, 0xfceb7c, 0xfeeaa6,
            0x5abd62, 0x5bbe63, 0x6bc467, 0x84ce6b, 0xa2d571, 0xc4de77, 0xffec84, 0xffedae,
            0x5fbc69, 0x5fbd69, 0x6fc36e, 0x86cb74, 0xa0d377, 0xc9df82, 0xfbec8f, 0xffebb5,
            0x64bd70, 0x64bd6f, 0x72c273, 0x89cc79, 0xa2d17f, 0xd6df8a, 0xfbea99, 0xffeec2,
            0x6bbe76, 0x68bd75, 0x76c279, 0x8dcc7e, 0xa4d385, 0xdee395, 0xffeda1, 0xfff0ca,
            0x6ebc7b, 0x6fbe7a, 0x79c17e, 0x91cb85, 0x9acf87, 0xdfe29c, 0xfceaa3, 0xfff3da,
            0x73bb81, 0x73bd80, 0x7bbf83, 0x91c889, 0xc1da98, 0xe6e3a4, 0xffe9ac, 0xfffbe6,
            0x78bd84, 0x76b986, 0x7bbc87, 0x86c38a, 0xd0dda3, 0xf0e9b2, 0x02f1c6, 0xfff6de};


    /**
     * A 3D lookup table give an instant camera look
     */
    public static final int[] LUT_INSTANT = {
            0x040305, 0x1b0506, 0x450909, 0x800e0c, 0xba140e, 0xda190e, 0xe01c0f, 0xe2200f,
            0x050622, 0x1c0823, 0x460b25, 0x821128, 0xbc162a, 0xdb1b2b, 0xe21e29, 0xe32127,
            0x060a46, 0x1d0c48, 0x470f4a, 0x83144d, 0xbf1b51, 0xde1e52, 0xe52250, 0xe6264f,
            0x060f6a, 0x1d116c, 0x48156e, 0x851a72, 0xc01f75, 0xe12377, 0xe82777, 0xe82a76,
            0x09168b, 0x1b168c, 0x461a8f, 0x841f92, 0xc22597, 0xe32998, 0xeb2c98, 0xec3098,
            0x0b1ca8, 0x171da9, 0x4521ac, 0x8325b0, 0xc22bb4, 0xe52eb5, 0xed32b6, 0xee35b6,
            0x0c23ba, 0x1424bc, 0x4027be, 0x7f2cc2, 0xc031c6, 0xe636c8, 0xee39ca, 0xef3cc9,
            0x0d2bbf, 0x0e2abf, 0x3a2dc3, 0x7b32c7, 0xbe38cb, 0xe53cce, 0xef3fcf, 0xef44ce,
            0x061207, 0x1a1608, 0x44190a, 0x801f0d, 0xbb2410, 0xda280f, 0xe12a10, 0xe22d10,
            0x071722, 0x1c1924, 0x461c26, 0x812129, 0xbd262c, 0xdc2a2c, 0xe32e2a, 0xe43027,
            0x081b46, 0x1d1d49, 0x481f4b, 0x84254e, 0xbe2a51, 0xdf2e52, 0xe63051, 0xe73451,
            0x08206a, 0x1d206c, 0x48246f, 0x842872, 0xc12e76, 0xe23277, 0xe93578, 0xea3877,
            0x0a258b, 0x1b268d, 0x472990, 0x842e93, 0xc23296, 0xe43798, 0xeb3a99, 0xec3d98,
            0x0c2ba8, 0x172ba9, 0x442fad, 0x8334b1, 0xc339b4, 0xe53db7, 0xed40b6, 0xef44b7,
            0x0d32ba, 0x1533bc, 0x3f35bf, 0x7f3bc3, 0xc241c8, 0xe643c9, 0xef46c9, 0xf14bca,
            0x0f39bf, 0x0e39bf, 0x393cc4, 0x7b41c8, 0xbe47cd, 0xe74cce, 0xf04fcf, 0xf152d0,
            0x092e0b, 0x1c330e, 0x43390f, 0x804111, 0xba4513, 0xda4b14, 0xe04d14, 0xe25014,
            0x0a3424, 0x1f3626, 0x463d2a, 0x84432c, 0xbd482e, 0xdd4c2f, 0xe44e2c, 0xe5512a,
            0x0c3a46, 0x203d49, 0x48414c, 0x864550, 0xc04c53, 0xe14f55, 0xe85254, 0xe95652,
            0x0d4069, 0x20436c, 0x484570, 0x874a74, 0xc24f78, 0xe4547a, 0xeb5578, 0xeb5c79,
            0x0f478a, 0x1d478c, 0x474a90, 0x864f95, 0xc35599, 0xe5599a, 0xed5d9c, 0xef5e9a,
            0x114da8, 0x1b4ea9, 0x4551ae, 0x8655b3, 0xc459b5, 0xe860b9, 0xef65b9, 0xf165ba,
            0x1254ba, 0x1754bb, 0x4056c0, 0x815ac5, 0xc260c9, 0xe866cb, 0xf268ca, 0xf36bcc,
            0x145bbe, 0x135bbe, 0x3b5dc3, 0x7c62ca, 0xbf68cf, 0xe86ed0, 0xf16dd1, 0xf272d1,
            0x0f5b12, 0x205e15, 0x446617, 0x7f7119, 0xb9791b, 0xd97e1b, 0xe07f1c, 0xe2821c,
            0x12612a, 0x24622d, 0x486a31, 0x817434, 0xbc7b35, 0xdc8134, 0xe48333, 0xe48330,
            0x14694a, 0x256b4d, 0x4c6f52, 0x857956, 0xc08059, 0xe28359, 0xe98759, 0xe98958,
            0x15706c, 0x24726d, 0x4c7672, 0x877e78, 0xc4857b, 0xe6887c, 0xed897c, 0xef8c7c,
            0x18778a, 0x23788c, 0x4c7d91, 0x868197, 0xc4889c, 0xe88c9e, 0xf0909f, 0xf0909c,
            0x197da7, 0x217ea8, 0x4983ae, 0x8688b5, 0xc58db8, 0xe992bb, 0xf394bb, 0xf497bc,
            0x1b83b9, 0x1d84ba, 0x4688be, 0x818dc7, 0xc693cc, 0xea97ce, 0xf49ace, 0xf49acf,
            0x1d89be, 0x1d88be, 0x408dc2, 0x7d93cb, 0xc298d2, 0xeb9cd3, 0xf5a0d5, 0xf6a2d4,
            0x17891a, 0x238b1c, 0x48951f, 0x7fa221, 0xb8ab23, 0xdab025, 0xe2b125, 0xe1b426,
            0x1b8e31, 0x279034, 0x4c9739, 0x82a53c, 0xbcaf3d, 0xdcb13d, 0xe4b63c, 0xe5b639,
            0x1e9751, 0x289953, 0x509c59, 0x84a85e, 0xbeb260, 0xe2b762, 0xe9b860, 0xe9bb60,
            0x1fa071, 0x28a172, 0x51a577, 0x89ac80, 0xc2b783, 0xe5bb84, 0xedbd83, 0xefbe82,
            0x20a790, 0x27a890, 0x4fac94, 0x8bb39b, 0xc4bca1, 0xe8bfa2, 0xf2c1a3, 0xf3c2a1,
            0x22adaa, 0x25adab, 0x4eb3b0, 0x88b9b6, 0xc6bfbc, 0xebc4be, 0xf5c6c0, 0xf6c8c0,
            0x24b2bc, 0x24b2bc, 0x49b6c0, 0x85bdc7, 0xc7c7d0, 0xedc8d2, 0xf7c8d1, 0xf8cdd3,
            0x25b7c1, 0x25b5c1, 0x43b8c4, 0x82c1cb, 0xc6cad4, 0xeccbd7, 0xf7cbd7, 0xf8ced8,
            0x1dab1f, 0x24ac20, 0x48b424, 0x7dc226, 0xb9ce27, 0xdcd22a, 0xe4d42b, 0xe5d42b,
            0x20b037, 0x28b138, 0x4cb73d, 0x81c440, 0xbcd141, 0xded643, 0xe6d741, 0xe6d63c,
            0x24b856, 0x2ab856, 0x51bc5c, 0x84c663, 0xc0d565, 0xe2d967, 0xeadb67, 0xecdb65,
            0x25c175, 0x2ac175, 0x52c47b, 0x88ca83, 0xc2d988, 0xe6dd8a, 0xefe08a, 0xeedf89,
            0x26c892, 0x27c792, 0x50cc97, 0x8bd09f, 0xc4dba7, 0xeae2a8, 0xf2e2a9, 0xf3e3a8,
            0x27cdad, 0x27cdad, 0x4dd1b1, 0x8ad9b9, 0xc8dfc3, 0xece5c4, 0xf5e7c5, 0xf6e6c3,
            0x28d0bf, 0x28d1c0, 0x47d4c3, 0x86dbca, 0xcae2d1, 0xece9d5, 0xf7ead5, 0xf8e9d6,
            0x2ad3c4, 0x29d2c4, 0x42d4c7, 0x7edccd, 0xc5e3d4, 0xedeada, 0xf8eada, 0xf8ebdc,
            0x1fb520, 0x1fb51f, 0x43bd24, 0x7acc28, 0xb7d829, 0xdddd2c, 0xe5de2d, 0xe6df2d,
            0x22ba37, 0x21ba36, 0x48c03d, 0x7fce41, 0xbada41, 0xdfdf42, 0xe7e140, 0xe7e13b,
            0x26c257, 0x24c256, 0x4bc55d, 0x84d165, 0xbddf66, 0xe3e467, 0xebe666, 0xede567,
            0x28cb76, 0x27ca75, 0x4ccd7b, 0x89d486, 0xc1e28a, 0xe6e88b, 0xf0e98c, 0xefe98a,
            0x27d194, 0x27d194, 0x4ad497, 0x88d9a0, 0xc3e4a7, 0xeaeba8, 0xf3eda9, 0xf5edaa,
            0x29d7b0, 0x29d7b0, 0x48dbb2, 0x85e0b8, 0xc7e6c4, 0xeceec5, 0xf7f1c6, 0xf8f0c6,
            0x2adac0, 0x2adac0, 0x40ddc2, 0x84e4ca, 0xc3ead0, 0xedf1d6, 0xf9f3d8, 0xfaf2d5,
            0x29dbc4, 0x29dac4, 0x3dddc7, 0x7be3ce, 0xc3ebd5, 0xecf1db, 0xf8f3db, 0xf9f2dc,
            0x1fb71e, 0x1fb71e, 0x3bbc22, 0x73cc27, 0xb4d92a, 0xdcdf2c, 0xe5e02d, 0xe6df2d,
            0x23bb36, 0x23bb36, 0x3ebf39, 0x78ce3f, 0xb5dc40, 0xdee23f, 0xe7e23e, 0xe7e13a,
            0x26c456, 0x26c456, 0x42c65a, 0x7cd063, 0xb7de65, 0xe1e668, 0xebe766, 0xece765,
            0x27cd75, 0x27cd75, 0x42ce79, 0x81d484, 0xbde28a, 0xe5eb8a, 0xeeeb8a, 0xefeb88,
            0x28d394, 0x28d392, 0x40d596, 0x82db9f, 0xbce4a7, 0xe9eeaa, 0xf3eea9, 0xf4eeaa,
            0x2ad8ae, 0x2ad8b0, 0x3ddab1, 0x80e1b8, 0xc2e6c2, 0xebefc5, 0xf6f1c4, 0xf7f0c6,
            0x2adbc0, 0x2adbc0, 0x39dcc1, 0x7be4cb, 0xc1ebd0, 0xeaf2d6, 0xf7f4d7, 0xf9f3d8,
            0x2adcc4, 0x2adcc4, 0x34ddc6, 0x76e5cd, 0xc1ebd6, 0xeaf1dc, 0xf8f4db, 0xf9f4dd};
    /**
     * A 3D lookup table give an cross Process film look
     */
    public static final int[] LUT_X_PROCESS = {
            0x000b0c, 0x000d0c, 0x00110b, 0x5f1709, 0xb91e06, 0xe62601, 0xf82f00, 0xff3900,
            0x000f43, 0x001142, 0x001542, 0x5f1940, 0xb8203d, 0xe6283a, 0xf83035, 0xff3930,
            0x001779, 0x00197a, 0x001b78, 0x5c1f78, 0xb82678, 0xe62c75, 0xf73473, 0xff3d72,
            0x0021ab, 0x0022ac, 0x0025ac, 0x5b29ac, 0xb62eaa, 0xe633a9, 0xf73aa9, 0xff42a8,
            0x002cd1, 0x002dd1, 0x002fd1, 0x5632d1, 0xb537d1, 0xe63cd1, 0xf742d0, 0xff4ad0,
            0x0039ea, 0x0039ea, 0x003cea, 0x503fea, 0xb241ea, 0xe445e9, 0xf74be9, 0xfe51e9,
            0x0046f6, 0x0047f6, 0x0049f7, 0x454af7, 0xae4cf7, 0xe351f7, 0xf654f7, 0xfe5bf7,
            0x0054fe, 0x0054fe, 0x0055fe, 0x3b57fe, 0xaa58fe, 0xe15cfe, 0xf75ffe, 0xfe65fe,
            0x003e0c, 0x003f0c, 0x00400a, 0x5e4309, 0xb84606, 0xe64a00, 0xf74f00, 0xff5400,
            0x004043, 0x004042, 0x004241, 0x5c4440, 0xb8483d, 0xe64c39, 0xf85235, 0xff562f,
            0x00437a, 0x00447a, 0x004579, 0x5b4778, 0xb64a77, 0xe64e74, 0xf75273, 0xff5971,
            0x0048ac, 0x0048ab, 0x004aac, 0x584caa, 0xb64faa, 0xe552a9, 0xf757a9, 0xff5aa8,
            0x004ed1, 0x004fd1, 0x0050d1, 0x5452d1, 0xb453d1, 0xe657d1, 0xf75cd0, 0xff5fd0,
            0x0056ea, 0x0056ea, 0x0057ea, 0x4d5aea, 0xb25cea, 0xe460e9, 0xf662e9, 0xfe68e9,
            0x005ff6, 0x0060f7, 0x0061f6, 0x4462f7, 0xaf64f7, 0xe266f7, 0xf669f7, 0xfe70f7,
            0x0069fe, 0x006afe, 0x006afe, 0x3b6bfe, 0xaa6dfe, 0xe26ffe, 0xf773fe, 0xfe77fe,
            0x00740c, 0x00740b, 0x00750a, 0x5e7608, 0xb77704, 0xe77b01, 0xf77c00, 0xff7f00,
            0x007442, 0x007542, 0x007541, 0x5e773e, 0xb7783c, 0xe77b39, 0xf77c34, 0xff802e,
            0x007579, 0x007779, 0x007778, 0x5b7877, 0xb67a77, 0xe67c75, 0xf77f74, 0xff8271,
            0x0078ac, 0x0079ab, 0x0079aa, 0x587baa, 0xb57caa, 0xe57eaa, 0xf77fa7, 0xff85a8,
            0x007cd1, 0x007cd1, 0x007cd1, 0x547ed1, 0xb37fd1, 0xe482d1, 0xf785d1, 0xfe86d0,
            0x0081ea, 0x0081ea, 0x0082ea, 0x4f82ea, 0xb183e9, 0xe586ea, 0xf68ae9, 0xfe8ce9,
            0x0086f7, 0x0086f7, 0x0086f6, 0x4586f7, 0xab88f7, 0xe38cf7, 0xf68cf7, 0xff90f7,
            0x008dfe, 0x008dfe, 0x008dfe, 0x398ffe, 0xa88ffe, 0xe292fe, 0xf592fd, 0xfe95fe,
            0x00a50b, 0x00a50a, 0x00a509, 0x5aa606, 0xb6a803, 0xffffff, 0xf7aa00, 0xffad00,
            0x00a642, 0x00a541, 0x00a63f, 0x58a73e, 0xb6a83c, 0xe6a939, 0xf7aa33, 0xffac2c,
            0x00a678, 0x00a679, 0x00a778, 0x58a877, 0xb4a977, 0xe5a973, 0xf7ac73, 0xffad71,
            0x00a8ac, 0x00a8aa, 0x00a8aa, 0x54a9aa, 0xb4aaaa, 0xe4aca9, 0xf6aca8, 0xfeaea5,
            0x00aad1, 0x00a9d1, 0x00aad1, 0x4daad1, 0xb0acd1, 0xe4add1, 0xf6aed1, 0xfeb0ce,
            0x00acea, 0x00adea, 0x00adea, 0x49adea, 0xaeaee8, 0xe3b0e9, 0xf6b2e8, 0xfeb3e9,
            0x00aff7, 0x00b0f7, 0x00b0f6, 0x3db0f7, 0xadb2f7, 0xe1b3f7, 0xf7b4f7, 0xfeb4f7,
            0x00b3fe, 0x00b3fe, 0x00b4fe, 0x32b4fe, 0xa8b5fe, 0xe2b8fe, 0xf5b9fe, 0xfeb9fe,
            0x00cc09, 0x00cc08, 0x00cd07, 0x50cd04, 0xb2ce01, 0xe4ce00, 0xf7cf00, 0xffd000,
            0x00cc3f, 0x00cd40, 0x00cd3d, 0x50cd3d, 0xb2ce39, 0xe5ce36, 0xf7cf32, 0xfed02c,
            0x00cd78, 0x00ce78, 0x00cd77, 0x4dcd77, 0xb0ce74, 0xe5cf75, 0xf6cf71, 0xfed16f,
            0x00ceab, 0x00ceaa, 0x00ceaa, 0x4acfaa, 0xb0cfa9, 0xe4cea9, 0xf6d0a7, 0xfed1a5,
            0x00cfd1, 0x00ced1, 0x00cfd1, 0x46d0d1, 0xaed0d1, 0xe3d0cf, 0xf6d1d0, 0xfed0ce,
            0x00d1ea, 0x00d0ea, 0x00d1ea, 0x3ed1e9, 0xacd1ea, 0xe1d0e8, 0xf7d2e9, 0xfed3e9,
            0x00d1f7, 0x00d1f7, 0x00d0f7, 0x32d2f7, 0xabd3f7, 0xe1d3f7, 0xf5d3f7, 0xfed5f7,
            0x00d5fe, 0x00d3fe, 0x00d3fe, 0x2dd4fe, 0xa6d6fe, 0xded5fe, 0xf6d5fd, 0xfed6fd,
            0x00e606, 0x00e606, 0x00e604, 0x45e601, 0xffffff, 0xe3e700, 0xf6e700, 0xfee700,
            0x00e63e, 0x00e63d, 0x00e63c, 0x45e639, 0xafe739, 0xe4e735, 0xf6e731, 0xfee725,
            0x00e677, 0x00e677, 0x00e675, 0x43e574, 0xaee774, 0xe2e773, 0xf6e771, 0xfee86e,
            0x00e7aa, 0x00e7aa, 0x00e7aa, 0x3ee7a9, 0xace7a9, 0xe3e7a9, 0xf6e8a8, 0xfee7a6,
            0x00e7d1, 0x00e7d1, 0x00e7d1, 0x3ae7d1, 0xaae6d1, 0xe2e8cf, 0xf7e7d0, 0xfee9ce,
            0x00e7e9, 0x00e7e9, 0x00e7e9, 0x37e9ea, 0xa8e9ea, 0xe1e9e9, 0xf5e9e9, 0xfee8e9,
            0x00e9f7, 0x00e9f7, 0x00e9f7, 0x2de8f7, 0xa6e8f7, 0xdee8f7, 0xf6eaf7, 0xfee9f7,
            0x00eafe, 0x00eafe, 0x00e9fe, 0x1beafe, 0x9feafe, 0xdee9fe, 0xf6ebfe, 0xfeebfe,
            0x00f502, 0x00f502, 0x00f500, 0xffffff, 0xaaf500, 0xe3f500, 0xf7f500, 0xfef600,
            0x00f53a, 0x00f53a, 0x00f539, 0x3df539, 0xaaf535, 0xe2f531, 0xf7f42c, 0xfef623,
            0x00f576, 0x00f575, 0x00f575, 0x3df574, 0xa9f573, 0xe2f571, 0xf5f46e, 0xfef46e,
            0x00f5a9, 0x00f5a9, 0x00f5a9, 0x3bf5aa, 0xa8f4a9, 0xe0f4a8, 0xf7f6a9, 0xfef5a6,
            0x00f5d1, 0x00f5d1, 0x00f5cf, 0x33f4d1, 0xa5f4cf, 0xe0f4d0, 0xf5f6ce, 0xfef6cf,
            0x00f6ea, 0x00f5ea, 0x00f6e9, 0x2cf5e9, 0xa4f6ea, 0xdef6e9, 0xf6f5e9, 0xfef5e9,
            0x00f6f7, 0x00f6f7, 0x00f6f7, 0x25f5f7, 0x9af5f7, 0xdff5f7, 0xf6f5f7, 0xfdf5f6,
            0x00f5fe, 0x00f5fe, 0x00f5fe, 0x14f5fe, 0x9af5fe, 0xdcf7fe, 0xf4f7fc, 0xfef6fd,
            0xffffff, 0xffffff, 0x00fc00, 0x2dfc00, 0xa4fc00, 0xe0fc00, 0xf5fc00, 0xfefc00,
            0x00fc38, 0x00fc38, 0x00fc35, 0x2efc34, 0xa3fc32, 0xe0fc2d, 0xf6fc25, 0xfefd1e,
            0x00fc74, 0x00fc75, 0x00fc73, 0x2afc73, 0x9ffc70, 0xdffc72, 0xf5fc6d, 0xfefc6c,
            0x00fca8, 0x00fca8, 0x00fca8, 0x28fca9, 0xa1fca8, 0xdffca8, 0xf6fba5, 0xfefca3,
            0x00fcd1, 0x00fcd0, 0x00fcd0, 0x25fccf, 0x99fccf, 0xdefed0, 0xf6fecf, 0xfefdcf,
            0x00fce8, 0x00feea, 0x00fdea, 0x20fee9, 0x9cfee9, 0xdcfde9, 0xf3fde9, 0xfdfdea,
            0x00fdf7, 0x00fef7, 0x00fef7, 0x14fdf7, 0x96fdf7, 0xdbfdf7, 0xf5fdf7, 0xfdfdf5,
            0x00fdfe, 0x00fdfe, 0x00fdfe, 0x0dfdfe, 0x94fdfe, 0xd9fdfc, 0xf2fdfc, 0xfdfdfd};
}
