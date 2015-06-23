package com.android.rs.infiniteLoop;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.os.Bundle;
import android.widget.ImageView;
import android.renderscript.*;

import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

public class MainActivity extends Activity {
    /* Number of bitmaps that is used for renderScript thread and UI thread synchronization.
       Ideally, this can be reduced to 2, however in some devices, 2 buffers still showing tierings on UI.
       Investigating a root cause.
     */
    private final int NUM_BITMAPS = 3;
    private int mCurrentBitmap = 0;
    private Bitmap mBitmapIn;
    private Bitmap[] mBitmapsOut;
    private ImageView mImageView;
    private Random mRand;

    private RenderScript mRS;
    private Allocation mInAllocation;
    private Allocation[] mOutAllocations;
    private ScriptC_infiniteLoop mScript;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main_layout);
        mRand = new Random();

        /*
         * Initialize UI
         */
        mBitmapIn = Bitmap.createBitmap(500, 500, Bitmap.Config.ARGB_8888);
        mBitmapsOut = new Bitmap[NUM_BITMAPS];
        for (int i = 0; i < NUM_BITMAPS; ++i) {
            mBitmapsOut[i] = Bitmap.createBitmap(mBitmapIn.getWidth(),
                    mBitmapIn.getHeight(), mBitmapIn.getConfig());
        }

        mImageView = (ImageView) findViewById(R.id.imageView);
        mImageView.setImageBitmap(mBitmapsOut[mCurrentBitmap]);
        mCurrentBitmap += (mCurrentBitmap + 1) % NUM_BITMAPS;

        Timer t = new Timer();
        t.schedule(new TimerTask() {
            @Override
            public void run() {
                updateImage(1.f);
            }
        }, 2000, 2000);

        /*
         * Create renderScript
         */
        createScript();

        /*
         * Invoke renderScript kernel and update imageView
         */
        updateImage(1.0f);
    }

    /*
     * Initialize RenderScript
     * In the sample, it creates RenderScript kernel that performs saturation manipulation.
     */
    private void createScript() {
        //Initialize RS
        mRS = RenderScript.create(this);

        //Allocate buffers
        mInAllocation = Allocation.createFromBitmap(mRS, mBitmapIn);
        mOutAllocations = new Allocation[NUM_BITMAPS];
        for (int i = 0; i < NUM_BITMAPS; ++i) {
            mOutAllocations[i] = Allocation.createFromBitmap(mRS, mBitmapsOut[i]);
        }

        //Load script
        mScript = new ScriptC_infiniteLoop(mRS);
    }

    /*
     * In the AsyncTask, it invokes RenderScript intrinsics to do a filtering.
     * After the filtering is done, an operation blocks at Allication.copyTo() in AsyncTask thread.
     * Once all operation is finished at onPostExecute() in UI thread, it can invalidate and update ImageView UI.
     */
    private class RenderScriptTask extends AsyncTask<Float, Integer, Integer> {
        Boolean issued = false;

        protected Integer doInBackground(Float... values) {
            int index = -1;
            if (isCancelled() == false) {
                issued = true;
                index = mCurrentBitmap;

                /*
                 * Set global variable in RS
                 */
                mScript.set_gColour(new Float4(mRand.nextFloat(), mRand.nextFloat(),
                        mRand.nextFloat(), 1.f));

                /*
                 * Invoke saturation filter kernel
                 */
                mScript.forEach_simple_kernel(mInAllocation, mOutAllocations[index]);

                /*
                 * Copy to bitmap and invalidate image view
                 */
                mOutAllocations[index].copyTo(mBitmapsOut[index]);
                mCurrentBitmap = (mCurrentBitmap + 1) % NUM_BITMAPS;
            }
            return index;
        }

        void updateView(Integer result) {
            if (result != -1) {
                // Request UI update
                mImageView.setImageBitmap(mBitmapsOut[result]);
                mImageView.invalidate();
            }
        }

        protected void onPostExecute(Integer result) {
            updateView(result);
        }

        protected void onCancelled(Integer result) {
            if (issued) {
                updateView(result);
            }
        }
    }

    RenderScriptTask currentTask = null;

    /*
    Invoke AsynchTask and cancel previous task.
    When AsyncTasks are piled up (typically in slow device with heavy kernel),
    Only the latest (and already started) task invokes RenderScript operation.
     */
    private void updateImage(final float f) {
        if (currentTask != null)
            currentTask.cancel(false);
        currentTask = new RenderScriptTask();
        currentTask.execute(f);
    }
}
