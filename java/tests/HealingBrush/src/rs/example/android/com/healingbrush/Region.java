/*
 * Copyright (C) 2015 The Android Open Source Project
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

package rs.example.android.com.healingbrush;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PathMeasure;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.support.v8.renderscript.Allocation;
import android.support.v8.renderscript.Element;
import android.support.v8.renderscript.RenderScript;
import android.support.v8.renderscript.Script;
import android.support.v8.renderscript.Type;
import android.util.Log;

import com.example.android.rs.sample.ScriptC_find_region;
import com.example.android.rs.sample.ScriptC_healing;

/**
 * Created by jiale on 4/26/2015.
 */
public class Region {
    private static final String TAG = "Region";
    int mPasteOffX; // offset to the start the bounding box of the ROI
    int mPasteOffY; // offset to the start the bounding box of the ROI
    int mCutOffsetX; // image coords of the cut  (mPointsXY - mPasteOffX + mCutOffsetX)
    int mCutOffsetY; // image coords of the cut (mPointsXY - mPasteOffY + mCutOffsetY)
    int mWidth;  // mWidth of bounding box of the ROI
    int mHeight; // mHeight of the bounding box of the ROI
    int[] mPaste; // contains a copy where to paste

    float[] mPointsXY; // polygon point in original image coordnates
    int numberOfPoints;

    Rect mSearchRange; // range to search in (original image coordinates
    Bitmap mMaskBitmap;
    Bitmap mOutput;

    public Region(float[] xy, Bitmap img) {
        mPointsXY = xy;

        RectF mRect = calcBounds(xy);
        int imgWidth = img.getWidth();
        int imgHeight = img.getHeight();

        mWidth = (((int) (8 + mRect.width())) & ~3);
        mHeight = (((int) (8 + mRect.height())) & ~3);

        mPaste = new int[mWidth * mHeight];
        mPasteOffX = (int) mRect.left - 1;
        mPasteOffY = (int) mRect.top - 1;
        if (mHeight <= 2 ||
                mPasteOffX < 0
                || mPasteOffY < 0
                || (mPasteOffX + mWidth) >= imgWidth
                || (mPasteOffY + mHeight) >= imgHeight) {
            throw new RuntimeException("ROI to close to the edge of the image");
        }

        mMaskBitmap = buildMask(mWidth, mHeight, mPasteOffX, mPasteOffY, mPointsXY);
        img.getPixels(mPaste, 0, mWidth, mPasteOffX, mPasteOffY, mWidth, mHeight);

        mSearchRange = calcSearchRange(imgWidth, imgHeight);
        Log.v(TAG, "done " + mWidth + "," + mHeight);

    }

    private static Bitmap buildMask(int width, int height, int offx, int offy, float[] xy) {
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ALPHA_8);

        Canvas c = new Canvas(bitmap);
        Paint paint = new Paint();
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(Color.BLACK);
        Path path = new Path();
        for (int i = 0; i < xy.length; i += 2) {
            if (i == 0) {
                path.moveTo(xy[i] - offx, xy[i + 1] - offy);
            } else {
                path.lineTo(xy[i] - offx, xy[i + 1] - offy);
            }
        }
        path.close();
        c.drawPath(path, paint);
        return bitmap;
    }

    private static int calcMaskArea(Allocation mask) {
        int w = mask.getType().getX();
        int h = mask.getType().getY();
        byte[] data = new byte[w * h];
        mask.copyTo(data);
        int count = 0;
        int val = data[0];
        for (int i = 0; i < data.length; i++) {
            if (data[i] != val) {
                count++;
            }

        }
        return count;
    }

    Drawable getSourceLocation() {
        final Path path = new Path();
        for (int i = 0; i < mPointsXY.length; i += 2) {
            if (i == 0) {
                path.moveTo(mPointsXY[i] - mPasteOffX + mCutOffsetX, mPointsXY[i + 1] - mPasteOffY + mCutOffsetY);
            } else {
                path.lineTo(mPointsXY[i] - mPasteOffX + mCutOffsetX, mPointsXY[i + 1] - mPasteOffY + mCutOffsetY);
            }
        }
        path.close();
        Drawable d = new Drawable() {
            Paint paint1 = new Paint();
            Paint paint2 = new Paint();

            {
                paint1.setStyle(Paint.Style.STROKE);
                paint2.setStyle(Paint.Style.STROKE);
                paint1.setColor(Color.BLACK);
                paint1.setStrokeWidth(2);
                paint2.setColor(Color.BLUE);

            }

            @Override
            public void draw(Canvas canvas) {

                canvas.drawPath(path, paint1);
                canvas.drawPath(path, paint2);

            }

            @Override
            public void setAlpha(int alpha) {

            }

            @Override
            public void setColorFilter(ColorFilter cf) {

            }

            @Override
            public int getOpacity() {
                return 0;
            }
        };
        return d;
    }

    private Rect calcSearchRange(int imgWidth, int imgHeight) {
        int xmin = Math.max(0, (int) (mPasteOffX - mWidth * 2));
        int ymin = Math.max(0, (int) (mPasteOffY - mHeight * 2));
        int xmax = (int) (mPasteOffX + mWidth * 3);
        int ymax = (int) (mPasteOffY + mHeight * 3);
        xmax = Math.min(imgWidth, xmax);
        ymax = Math.min(imgHeight, ymax);
        xmax = Math.max(0, xmax);
        ymax = Math.max(0, ymax);
        return new Rect(xmin, ymin, xmax, ymax);
    }

    RectF calcBounds(float[] xy) {
        float minx = xy[0], miny = xy[1];
        float maxx = xy[0], maxy = xy[1];
        for (int i = 0; i < xy.length; i += 2) {
            minx = Math.min(minx, xy[i]);
            maxx = Math.max(maxx, xy[i]);
            miny = Math.min(miny, xy[i + 1]);
            maxy = Math.max(maxy, xy[i + 1]);
        }
        RectF rect = new RectF();
        rect.set(minx, miny, maxx, maxy);
        return rect;
    }

    public Drawable findMatch(ScriptC_find_region findRegion, RenderScript mRs, Bitmap image) {
        long time = System.nanoTime();
        Allocation border_coords;
        Allocation border_values;

        Type.Builder builderU32_2 = new Type.Builder(mRs, Element.U32_2(mRs));
        builderU32_2.setX(mPointsXY.length / 2);
        border_coords = Allocation.createTyped(mRs, builderU32_2.create());
        int[] coords = new int[mPointsXY.length];
        for (int i = 0; i < coords.length; i++) {
            coords[i] = (int) mPointsXY[i];
        }
        border_coords.copy1DRangeFrom(0, coords.length / 2, coords);
        findRegion.set_border_coords(border_coords);

        findRegion.set_image(Allocation.createFromBitmap(mRs, image));


        Type.Builder builderF32_3 = new Type.Builder(mRs, Element.F32_3(mRs));
        builderF32_3.setX(mPointsXY.length / 2);
        border_values = Allocation.createTyped(mRs, builderF32_3.create());
        findRegion.set_border_values(border_values);
        findRegion.forEach_extractBorder(border_coords, border_values);


        Type.Builder builderF32 = new Type.Builder(mRs, Element.F32(mRs));
        builderF32.setX(mSearchRange.width());
        builderF32.setY(mSearchRange.height());

        Allocation fit = Allocation.createTyped(mRs, builderF32.create());
        findRegion.set_borderLength(mPointsXY.length / 2);
        int noSearch_x = mPasteOffX - mSearchRange.left;
        int noSearch_y = mPasteOffY - mSearchRange.top;
        findRegion.set_imagePosX(noSearch_x);
        findRegion.set_imagePosY(noSearch_y);
        Script.LaunchOptions options = new Script.LaunchOptions();
        options.setX(0, mSearchRange.width() - mWidth);
        options.setY(0, mSearchRange.height() - mHeight);
        findRegion.forEach_bordercorrelation(fit, options);


        Log.v(TAG, "noSearch " + noSearch_x + ", " + noSearch_y);
        Log.v(TAG, "noSearch " + mWidth + ", " + mHeight);


        float[] fitmap = new float[mSearchRange.width() * mSearchRange.height()];
        fit.copyTo(fitmap);
        float minFit = fitmap[0];
        int fit_x = 0;
        int fit_y = 0;
        int w = mSearchRange.width();
        int h = mSearchRange.height();
        int reg_minx = noSearch_x;
        int reg_miny = noSearch_y;
        int reg_width = mWidth;
        int reg_height = mHeight;
        int reg_maxx = reg_minx + reg_width;
        int reg_maxy = reg_miny + reg_height;
        for (int y = 0; y < h - mHeight; y++) {
            int yw = y * w;
            for (int x = 0; x < w - mWidth; x++) {
                if (!(x > reg_maxx || x + reg_width < reg_minx || y > reg_maxy
                        || y + reg_height < reg_miny)) {
                    continue;
                }
                float v = fitmap[x + yw];
                if (v < minFit) {
                    minFit = v;
                    fit_x = x;
                    fit_y = y;
                }
            }
        }
        mCutOffsetX = fit_x + mSearchRange.left;
        mCutOffsetY = fit_y + mSearchRange.top;

        Log.v(TAG, "best fit =  " + fit_x + ", " + fit_y);

        final Bitmap map = Bitmap.createBitmap(mSearchRange.width(), mSearchRange.height(), Bitmap.Config.ARGB_8888);
        int[] data = new int[mSearchRange.width() * mSearchRange.height()];

        float min = fitmap[0], max = fitmap[0];
        for (int i = 0; i < fitmap.length; i++) {
            min = Math.min(fitmap[i], min);
            max = Math.max(fitmap[i], max);
        }
        Log.v(TAG, "min,max = " + min + ", " + max);
        for (int i = 0; i < fitmap.length; i++) {
            int v = 255 - (int) ((fitmap[i] - min) * 255f / (max - min));
            data[i] = 0xFF000000 | v * 0x10101;
        }
        map.setPixels(data, 0, map.getWidth(), 0, 0, map.getWidth(), map.getHeight());
        final Path path = new Path();
        for (int i = 0; i < mPointsXY.length; i += 2) {
            if (i == 0) {
                path.moveTo(mPointsXY[i] - mPasteOffX + mCutOffsetX, mPointsXY[i + 1] - mPasteOffY + mCutOffsetY);
            } else {
                path.lineTo(mPointsXY[i] - mPasteOffX + mCutOffsetX, mPointsXY[i + 1] - mPasteOffY + mCutOffsetY);
            }
        }

        Drawable d = new Drawable() {
            Paint paint = new Paint();

            @Override
            public void draw(Canvas canvas) {
                canvas.drawPath(path, paint);
                // canvas.drawRect(mSearchRange, paint1);
                // canvas.drawBitmap(map,mSearchRange.left,mSearchRange.top, paint);
            }

            @Override
            public void setAlpha(int alpha) {

            }

            @Override
            public void setColorFilter(ColorFilter cf) {

            }

            @Override
            public int getOpacity() {
                return 0;
            }
        };
        Log.v(TAG, "Time to find replacement= " + (System.nanoTime() - time) / 1E6f + "ms");
        return d;
    }

    Bitmap createMutableBitmap(Bitmap image, int x, int y, int width, int height) {
        Bitmap ret = Bitmap.createBitmap(image, x, y, width, height);
        return ret.copy(Bitmap.Config.ARGB_8888, true);
    }

    /**
     * This function only assumes mPointsXY, mPasteOffX, mPasteOffY
     *
     * @param healing
     * @param rs
     * @param image
     */
    public void heal(ScriptC_healing healing, RenderScript rs, Bitmap image, Bitmap output) {
        long time = System.nanoTime();
        Type.Builder floatImage = new Type.Builder(rs, Element.F32_3(rs));
        floatImage.setX(mWidth);
        floatImage.setY(mHeight);


        Bitmap maskBitmap = buildMask(mWidth, mHeight, mPasteOffX, mPasteOffY, mPointsXY);

        Allocation dest1 = Allocation.createTyped(rs, floatImage.create());
        Allocation dest2 = Allocation.createTyped(rs, floatImage.create());
        healing.set_dest1(dest1);
        healing.set_dest2(dest2);

        Bitmap destBitmap = createMutableBitmap(image, mPasteOffX, mPasteOffY, mWidth, mHeight);
        Allocation dest_uc4 = Allocation.createFromBitmap(rs, destBitmap);
        healing.forEach_convert_to_f(dest_uc4, dest1);

        Bitmap src = createMutableBitmap(image, mCutOffsetX, mCutOffsetY, mWidth, mHeight);
        Allocation src_f3 = Allocation.createTyped(rs, floatImage.create());
        Allocation src_uc4 = Allocation.createFromBitmap(rs, src);
        healing.forEach_convert_to_f(src_uc4, src_f3);
        healing.set_src(src_f3);

        Allocation mask = Allocation.createFromBitmap(rs, maskBitmap);
        healing.set_mask(mask);
        // healing.set_src(src_uc4 = Allocation.createFromBitmap(rs, src));

        Allocation laplace_f3 = Allocation.createTyped(rs, floatImage.create());
        healing.set_laplace(laplace_f3);

        Script.LaunchOptions options = new Script.LaunchOptions();
        options.setX(1, mWidth - 1);
        options.setY(1, mHeight - 1);
        healing.forEach_laplacian(laplace_f3, options);
        healing.forEach_copyMasked(mask, dest1);
        int area = calcMaskArea(mask);

        int steps = (int) Math.sqrt(area);

        for (int i = 0; i < steps; i++) {
            healing.forEach_solve1(mask, dest2);
            healing.forEach_solve2(mask, dest1);
        }

        healing.forEach_convert_to_uc(dest1, dest_uc4);
        rs.finish();

        healing.forEach_alphaMask(dest_uc4, dest_uc4);
        rs.finish();

        dest_uc4.copyTo(destBitmap);
        rs.finish();
        destBitmap.setHasAlpha(true);
        rs.finish();

        Bitmap map = image.copy(Bitmap.Config.ARGB_8888, true);
        Canvas c = new Canvas(output);
        c.drawBitmap(image, 0, 0, null);
        c.drawBitmap(destBitmap, mPasteOffX, mPasteOffY, null);
        Log.v(TAG, " time to smart paste = " + (System.nanoTime() - time) / 1E6f + "ms");

    }
}
