package rs.example.android.com.effectsupdate;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.renderscript.Allocation;
import android.renderscript.RenderScript;
import android.renderscript.Sampler;
import android.util.Log;
import android.util.Size;
import android.view.View;
import android.widget.ImageView;

import com.android.rslib.ScriptC_PhotoEffects;


/**
 * Created by hoford on 3/25/15.
 */
public class FilteredImageView {
    private static final String TAG = "RtkFilteredImageView";
    ;
    private final RenderScript rs;
    PhotoEffects script;
    private ImageView imageView;
    private Bitmap mOutBitmap;
    Size imageViewSize;
    FilterTask filterTask;
    private Allocation mInputAllocation;
    private Bitmap mBitmap;
    int mFilterType = 0;
    Allocation mOutAllocation;

    float mParam1;
    float mParam2;
    float mParam3;
    boolean mQueFilter = false;

    public static final int FILTERTYPE_INVERT = 1;
    public static final int FILTERTYPE_EXPOSURE = 2;
    public static final int FILTERTYPE_CONTRAST = 3;
    public static final int FILTERTYPE_DISTORTION = 4;
    public static final int FILTERTYPE_GRAIN = 5;
    public static final int FILTERTYPE_SATURATE = 6;
    public static final int FILTERTYPE_TEMPERATURE = 7;
    public static final int FILTERTYPE_SHARPEN = 8;
    public static final int FILTERTYPE_HUE = 9;
    public static final int FILTERTYPE_LOOK = 10;
    public static final int FILTERTYPE_VIGNETTE = 11;
    public static final int FILTERTYPE_SHADOW = 12;
    public static final int FILTERTYPE_DEHAZE = 13;


    public FilteredImageView(ImageView view, Context ctx) {
        imageView = view;
        rs = RenderScript.create(ctx);
        script = new PhotoEffects(rs);
        view.addOnLayoutChangeListener(new View.OnLayoutChangeListener() {
            @Override
            public void onLayoutChange(View v, int left, int top, int right, int bottom, int oldLeft, int oldTop, int oldRight, int oldBottom) {
                if (right - left == 0 || bottom - top == 0) {
                    return;
                }
                if (right - left != oldRight - oldLeft || bottom - top != oldBottom - oldTop) {
                    onSizeAvailable(right - left, bottom - top);
                }
            }
        });
    }

    private void onSizeAvailable(int width, int height) {
        imageViewSize = new Size(width, height);
        render();
    }

    private void render() {
        if (filterTask == null) {
            mQueFilter = false;
            filterTask = new FilterTask();
            filterTask.execute(script);
        } else {
            mQueFilter = true;
        }
    }

    public void setFilterType(int type) {
        mFilterType = type;
        render();

    }

    public void setParameters(float v1, float v2, float v3) {
        mParam1 = v1;
        mParam2 = v2;
        mParam3 = v3;
        Log.v(TAG, "             " + v1 + " , " + v2 + " , " + v3);
        render();
    }

    private class FilterTask extends AsyncTask<PhotoEffects, Integer, Bitmap> {

        protected Bitmap doInBackground(PhotoEffects... script) {

            if (mOutBitmap == null) {
                mOutBitmap = Bitmap.createBitmap(mBitmap.getWidth(), mBitmap.getHeight(), mBitmap.getConfig());
                mInputAllocation = Allocation.createFromBitmap(rs, mBitmap);
                mOutAllocation = Allocation.createFromBitmap(rs, mOutBitmap);
            }
            PhotoEffects s = script[0];
            int w = mBitmap.getWidth();
            int h = mBitmap.getHeight();
            Log.v(TAG, "size=" + w + ", " + h);
            switch (mFilterType) {
                case FILTERTYPE_INVERT:
                    s.forEach_invert(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_EXPOSURE:
                    s.set_exposureStrength(mParam1 * 2 - 1);
                    s.forEach_exposure(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_CONTRAST:
                    s.set_contrastStrength(mParam1 * 2 - 1);
                    s.forEach_contrast(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_DISTORTION:
                    s.set_correction(mParam1 * 2 - 1, mParam2, 0);
                    s.forEach_distortion(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_GRAIN:
                    s.set_grainStrength(mParam1);
                    s.forEach_grain(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_SATURATE:
                    s.set_saturationStrength(mParam1);
                    s.forEach_saturation(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_TEMPERATURE:
                    s.set_temprature(mParam1 * 8000 + 2000);
                    s.forEach_temperature(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_SHARPEN:
                    s.set_sharpenStrength(mParam1);
                    s.forEach_sharpen(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_HUE:
                    s.set_hueValue(mParam1 * 360 - 180);
                    s.forEach_hue(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_LOOK:
                    s.forEach_look(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_SHADOW:
                    s.set_shadowsStrength((mParam1 * 2 - 1));
                    s.forEach_shadows(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_VIGNETTE:
                    s.set_vignetteExposure(mParam2 * 200 - 100);
                    s.set_vignetteRate(mParam1 * 100);
                    s.set_vignetteCenter(w / 2f, h / 2f);
                    s.set_vignetteRadius(w / 2, h / 2);
                    s.set_vignetteSaturation(0);
                    s.set_vignetteContrast(0);
                    s.forEach_vignette(mInputAllocation, mOutAllocation);
                    break;
                case FILTERTYPE_DEHAZE:
                    s.set_dehazeStrength((mParam1 * 2 - 1));
                    s.forEach_dehaze(mInputAllocation, mOutAllocation);
            }

            mOutAllocation.copyTo(mOutBitmap);

            return mOutBitmap;
        }

        protected void onProgressUpdate(Integer... progress) {
        }

        protected void onPostExecute(Bitmap result) {
            imageView.setImageBitmap(result);
            filterTask = null;
            if (mQueFilter) {
                render();
            }
        }
    }


    public void setImage(Bitmap bitmap) {
        mBitmap = bitmap;
    }

}
