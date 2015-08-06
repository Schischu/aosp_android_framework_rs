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

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;

import android.os.Bundle;
import android.support.v8.renderscript.RenderScript;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;

import com.example.android.rs.sample.ScriptC_find_region;
import com.example.android.rs.sample.ScriptC_healing;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.InputStream;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";
    ImageView mImgView;
    DrawView mOverlay;
    Matrix mMatrix = new Matrix();
    Matrix mInverseMatrix = new Matrix();
    Bitmap mDisplayedImage;
    Bitmap mImage2;
    RenderScript mRs;
    ScriptC_healing mHealingScript;
    ScriptC_find_region mFindRegion;
    private float mZoom = 0.8f;
    float mYOffset = 0;
    float mXOffset = 0;
    RunScript mRunScript = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mImgView = (ImageView) findViewById(R.id.imageview);
        mOverlay = (DrawView) findViewById(R.id.overlay);
        mOverlay.setImageView(mImgView);
        mRs = RenderScript.create(this.getBaseContext());
        mHealingScript = new ScriptC_healing(mRs);
        mFindRegion = new ScriptC_find_region(mRs);

        mImgView.setOnTouchListener(new View.OnTouchListener() {
            float[] imgPoint = new float[2];
            float[] imgMoveList = new float[100];
            boolean mPanZoomDown = false;

            float mCenterDownX;
            float mCenterDownY;
            float mDistDown;
            float mDownXOffset;
            float mDownYOffset;
            float mDownZoom;
            boolean inMultiTouch = false;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction();
                float x = event.getX();
                float y = event.getY();
                imgPoint[0] = x;
                imgPoint[1] = y;
                int sw = mImgView.getWidth();
                int sh = mImgView.getHeight();
                int iw = mImgView.getDrawable().getIntrinsicWidth();
                int ih = mImgView.getDrawable().getIntrinsicHeight();
                switch (action) {
                    case MotionEvent.ACTION_DOWN:
                        Log.v(TAG, "ACTION_DOWN " + event.getPointerCount());

                        break;
                    case MotionEvent.ACTION_UP:
                        Log.v(TAG, "ACTION_UP " + event.getPointerCount());

                        break;
                    case MotionEvent.ACTION_MOVE:
                        Log.v(TAG, "ACTION_MOVE " + event.getPointerCount());
                        break;
                }
                if (event.getPointerCount() > 1) {
                    inMultiTouch = true;
                }
                if (event.getPointerCount() == 2) {
                    float x1 = event.getX(0);
                    float y1 = event.getY(0);
                    float x2 = event.getX(1);
                    float y2 = event.getY(1);
                    if (mPanZoomDown) {
                        float dx = (x1 + x2) / 2 - mCenterDownX;
                        float dy = (y1 + y2) / 2 - mCenterDownY;
                        float zoom = (float) Math.hypot(x1 - x2, y1 - y2);
                        mZoom = zoom * mDownZoom / mDistDown;

                        float scale = mZoom * Math.min(sw / (float) iw, sh / (float) ih);
                        mXOffset = mDownXOffset + 2 * (dx / (sw - scale * iw));
                        mYOffset = mDownYOffset + 2 * (dy / (sh - scale * ih));
                        if (Math.abs(mXOffset) > 1) {
                            mXOffset = Math.signum(mXOffset);
                        }
                        if (Math.abs(mYOffset) > 1) {
                            mYOffset = Math.signum(mYOffset);
                        }
                    } else {
                        mOverlay.undo();
                        mPanZoomDown = true;
                        mCenterDownX = (x1 + x2) / 2;
                        mCenterDownY = (y1 + y2) / 2;
                        mDistDown = (float) Math.hypot(x1 - x2, y1 - y2);
                        mDownXOffset = mXOffset;
                        mDownYOffset = mYOffset;
                        mDownZoom = mZoom;
                    }
                } else {
                    if (mPanZoomDown) {
                        mPanZoomDown = false;
                    }
                }
                if (!mPanZoomDown) {
                    switch (action) {
                        case MotionEvent.ACTION_DOWN:
                            mInverseMatrix.mapPoints(imgPoint);
                            mOverlay.downPoint(imgPoint);
                            mOverlay.invalidate();

                            break;
                        case MotionEvent.ACTION_UP:
                            if (inMultiTouch && event.getPointerCount() == 1) {
                                inMultiTouch = false;
                            } else {
                                mInverseMatrix.mapPoints(imgPoint);
                                mOverlay.upPoint(imgPoint);
                                mOverlay.invalidate();
                            }

                            break;
                        case MotionEvent.ACTION_MOVE:

                            int size = event.getHistorySize();
                            size = Math.min(size, imgMoveList.length / 2);
                            for (int i = 0; i < size; i++) {
                                imgMoveList[i * 2] = event.getHistoricalX(size - i - 1);
                                imgMoveList[i * 2 + 1] = event.getHistoricalY(size - i - 1);
                            }
                            mInverseMatrix.mapPoints(imgMoveList, 0, imgMoveList, 0, size);
                            if (!inMultiTouch) {
                                mOverlay.movePoint(imgMoveList, size);
                                mOverlay.invalidate();
                            }
                            break;
                    }
                }
                updateMatrix();

                return true;
            }
        });
        Intent intent = getIntent();

        if (intent != null) {

            String s = intent.getType();
            if (s != null && s.indexOf("image/") != -1) {
                Uri data = (Uri) intent.getParcelableExtra(Intent.EXTRA_STREAM);
                if (data != null) {
                    InputStream input = null;
                    try {
                        input = getContentResolver().openInputStream(data);
                        mDisplayedImage = BitmapFactory.decodeStream(input);
                        Log.v(TAG, "BITMAP SIZE = " + mDisplayedImage.getWidth() + "," +
                                mDisplayedImage.getHeight());
                        mImgView.setImageBitmap(mDisplayedImage);
                        updateMatrix();
                        mImgView.invalidate();

                        return;
                    } catch (FileNotFoundException e) {
                        e.printStackTrace();
                    }

                }
            }
        }
        mDisplayedImage = getLocalImage();
        mImgView.setImageBitmap(mDisplayedImage);
        Log.v(TAG, "BITMAP SIZE = " + mDisplayedImage.getWidth() + "," +
                mDisplayedImage.getHeight());

        updateMatrix();
        mImgView.invalidate();
    }

    void updateMatrix() {
        int sw = mImgView.getWidth();
        int sh = mImgView.getHeight();
        int iw = mImgView.getDrawable().getIntrinsicWidth();
        int ih = mImgView.getDrawable().getIntrinsicHeight();


        mMatrix.reset();
        float scale = mZoom * Math.min(sw / (float) iw, sh / (float) ih);
        mMatrix.postTranslate((1 + mXOffset) * (sw - iw * scale) / 2,
                (1 + mYOffset) * (sh - ih * scale) / 2);
        mMatrix.preScale(scale, scale);
        boolean ret = mMatrix.invert(mInverseMatrix);
        if (!ret) {
            Log.e(TAG, "Fail to invert");
        }
        mImgView.setImageMatrix(mMatrix);
        mImgView.invalidate();
        mOverlay.invalidate();
    }

    void getScreenCoord(float[] point) {
        Matrix matrix = mImgView.getImageMatrix();
    }

    Bitmap getLocalImage() {

        File folder;
        folder = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        File[] files = folder.listFiles();
        Log.v(TAG, "files" + files.length);
        for (int i = 0; i < files.length; i++) {
            Log.v(TAG, "[" + i + "]=" + files[i].getAbsolutePath());
            if (files[i].getName().toLowerCase().endsWith(".jpg")) {
                Bitmap bitmap = BitmapFactory.decodeFile(files[i].getAbsolutePath());
                return bitmap;
            }
        }
        return null;
    }

    public void heal(View v) {
        Region r = mOverlay.getRegion(mDisplayedImage);
        if (mRunScript == null) {
            mRunScript = new RunScript();
            mRunScript.execute(r);
        }
    }

    public void undo(View v) {
        // TODO implement undo
    }

    public void redo(View v) {
        // TODO implement redo
    }

    public void touch(MotionEvent event) {

    }

    class RunScript extends AsyncTask<Region, String, String> {

        protected String doInBackground(Region... regions) {
            Drawable d = regions[0].findMatch(mFindRegion, mRs, mDisplayedImage);

            if (mImage2 == null) {
                mImage2 = mDisplayedImage.copy(Bitmap.Config.ARGB_8888, true);

            }
            regions[0].heal(mHealingScript, mRs, mImage2, mImage2);

            return "";
        }

        @Override
        protected void onPostExecute(String s) {
            super.onPostExecute(s);
            mOverlay.invalidate();
            mImgView.setImageBitmap(mImage2);
            mRunScript = null;
        }
    }

}
