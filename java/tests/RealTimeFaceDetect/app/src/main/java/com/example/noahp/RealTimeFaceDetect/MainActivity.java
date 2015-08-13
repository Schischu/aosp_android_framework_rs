package com.example.noahp.RealTimeFaceDetect;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

import org.opencv.android.CameraBridgeViewBase;
import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.imgproc.Imgproc;
import org.opencv.objdetect.CascadeClassifier;

import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.Spinner;
import android.widget.TextView;

import com.example.noahp.facialrecogrs.R;


public class MainActivity extends ActionBarActivity
        implements CameraBridgeViewBase.CvCameraViewListener,AdapterView.OnItemSelectedListener {

    // Constants for optimization/resize levels
    private final int FRAME_OPTI_SKIP = 1;
    private final int TARGET_WIDTH = 320;
    private MatOfRect mFaceDetects;
    // OpenCV camera + detector objects
    private CameraBridgeViewBase mOpenCvCameraView;
    private CascadeClassifier mDetector;
    // Sizes for constraining detection scales (detectMultiScale)
    private Size mMinSize;
    private Size mMaxSize;
    private int mCount;
    private Double mScale_factor;
    private long mTime;
    private double mFps;
    private TextView mFpsTv;
    private TextView mRenderTimeTv;
    private Spinner mHaarSpinner;
    private String mCascadeName;
    final Handler mUpdateUIHandler = new Handler();
    private List<String> mHaarCascades;
    private final String TAG = "FaceDetect";
    private final String DEFAULT_CASCADE = "haarcascade_frontalface_alt";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initOpenCV();
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_main);
        retrieveViews();
        initDMSComponents();
        loadClassifier();
        loadSpinner();
    }

    /**
     * Loads UI spinner with options for cascade classifiers.
     */
    private void loadSpinner() {
        mHaarCascades = new ArrayList<String>();
        Field[] fields = R.raw.class.getFields();
        for (Field f : fields)
            try {
                mHaarCascades.add(f.getName());
            } catch (IllegalArgumentException e) {
            }
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                this, android.R.layout.simple_spinner_item, mHaarCascades);
        mHaarSpinner.setAdapter(adapter);
        mHaarSpinner.setOnItemSelectedListener(this);
        int spinnerPosition = adapter.getPosition(DEFAULT_CASCADE);
        mHaarSpinner.setSelection(spinnerPosition);
    }

    /**
     * Handle selection of haar cascade classifier.
     *
     * @param parent the spinner of choices
     * @param view   the view selected
     * @param pos    the position of the haar cascade chosen
     * @param id     the associated id
     */
    public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
        if (mOpenCvCameraView != null && mOpenCvCameraView.isEnabled()) {
            mOpenCvCameraView.disableView();
        }
        mCascadeName = (String) parent.getItemAtPosition(pos);
        mDetector = new CascadeClassifier();
        mFaceDetects = new MatOfRect();
        loadClassifier();
        mOpenCvCameraView.enableView();
    }

    /**
     * Satisfy interface requirements.
     *
     * @param parent the parent spinner
     */
    public void onNothingSelected(AdapterView<?> parent) {
    }

    /**
     * Initialized face detection components: Camera View, Cascade Classifier, etc.
     */
    @SuppressWarnings("unchecked")
    private void initDMSComponents() {
        mDetector = new CascadeClassifier();
        mFaceDetects = new MatOfRect();
        mCount = 0;
        Log.d(TAG, "Camera view listener initiated");
        mOpenCvCameraView.setCvCameraViewListener(this);
        mOpenCvCameraView.enableView();
    }

    /**
     * Retrieve necessary TextViews and CheckBoxes.
     */
    private void retrieveViews() {
        mFpsTv = (TextView) findViewById(R.id.fpstext);
        mRenderTimeTv = (TextView) findViewById(R.id.dms);
        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.cameraview);
        mHaarSpinner = (Spinner) findViewById(R.id.spinner);
    }

    @Override
    public void onResume() {
        if (mOpenCvCameraView != null && !mOpenCvCameraView.isEnabled()) {
            mOpenCvCameraView.enableView();
        }
        super.onResume();
    }

    @Override
    public void onDestroy() {

        if (mOpenCvCameraView != null && mOpenCvCameraView.isEnabled()) {
            mOpenCvCameraView.disableView();
        }
    }

    /**
     * Populate CascadeClassifier with Haar feature training data.
     */
    private void loadClassifier() {
        if (mCascadeName == null) {
            mCascadeName = DEFAULT_CASCADE;
        }
        try {
            InputStream is = getResources().openRawResource(this.getResources().getIdentifier(mCascadeName, "raw", this.getPackageName()));
            File cascadeDir = getDir("cascade", MODE_PRIVATE);
            File mCascadeFile = new File(cascadeDir, mCascadeName + ".xml");
            FileOutputStream os = new FileOutputStream(mCascadeFile);

            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = is.read(buffer)) != -1) {
                os.write(buffer, 0, bytesRead);
            }
            is.close();
            os.close();

            if (!mDetector.load(mCascadeFile.getAbsolutePath())) {
                Log.d(TAG, "FAILED TO LOAD DETECTOR");
            } else {
                Log.d(TAG, "SUCCESSFULLY LOADED DETECTOR: " + mCascadeFile.getAbsolutePath());
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Load native openCV library.
     */
    @SuppressWarnings("SpellCheckingInspection")
    private static void initOpenCV() {
        System.loadLibrary("opencv_java");
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();


        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onCameraViewStarted(int width, int height) {
    }

    @Override
    public void onCameraViewStopped() {

    }

    /**
     * Updates UI with run times and FPS.
     */
    final Runnable updateFPS = new Runnable() {
        public void run() {
            mFpsTv.setText("FPS: " + Double.toString(mFps));
            mRenderTimeTv.setText("TOTAL: " + Double.toString(1.0 / mFps) + " sec");
        }
    };

    /**
     * Initiate frame processing, facial detection and rectangle drawing around detected faces.
     *
     * @param inputFrame the frame grabbed from real time stream.
     * @return fully processed frame to display to camera view.
     */
    @Override
    public Mat onCameraFrame(Mat inputFrame) {
        if (mScale_factor == null)
            mScale_factor = inputFrame.width() / (double) TARGET_WIDTH;
        return processFrameOptimized(inputFrame);
    }


    /**
     * Executes facial detection and frame processing without optimization.
     *
     * @param inputFrame the frame to be processed from the real time camera stream.
     * @return the processed input frame to display to camera view.
     */
    private Mat processFrameOptimized(Mat inputFrame) {
        boolean checkMinMax;
        mTime = System.currentTimeMillis();
        if (mFaceDetects.toArray().length == 0 | mCount % FRAME_OPTI_SKIP == 0 || mCount == 0) {
            checkMinMax = true;
            mMaxSize = new Size(0, 0);
            mMinSize = new Size(inputFrame.width(), inputFrame.height());
            Mat resizedImage = reduceImage(inputFrame);
            mDetector.detectMultiScale(resizedImage, mFaceDetects);
        } else {
            checkMinMax = false;
            Mat resizedImage = reduceImage(inputFrame);
            mDetector.detectMultiScale(resizedImage, mFaceDetects, 1.1, 3, 0, mMinSize, mMaxSize);
        }
        inputFrame = drawRects(checkMinMax, inputFrame);
        recalculateFPS();
        return inputFrame;
    }


    /**
     * Draws rectangles around each detected face onto the frame.
     *
     * @param checkRect  whether or not this method must update the min/max sizes of rectangles.
     * @param inputFrame the input frame to draw the rectangles on.
     * @return a frame with rectangles around detected faces.
     */
    private Mat drawRects(boolean checkRect, Mat inputFrame) {
        for (Rect rect : mFaceDetects.toArray()) {
            if (checkRect) {
                updateMinMax(rect);
            }
            Imgproc.rectangle(inputFrame,
                    new Point(mScale_factor * rect.x, mScale_factor * rect.y),
                    new Point(mScale_factor * rect.x + mScale_factor * rect.width,
                            mScale_factor * rect.y + mScale_factor * rect.height),
                    new Scalar(0, 255, 0), 4);
            if (checkRect) {
                mMaxSize = new Size(mMaxSize.width + .2 * mMaxSize.width,
                        mMaxSize.height + .2 * mMaxSize.height);
                mMinSize = new Size(mMinSize.width - .2 * mMinSize.width,
                        mMinSize.height - .2 * mMinSize.height);
            }
        }
        return inputFrame;
    }

    /**
     * Maintain max sized rectangle dimensions.
     *
     * @param rect the rectangle who's dimensions must be checked.
     */
    private void updateMinMax(Rect rect) {
        if (rect.size().area() > mMaxSize.area()) {
            mMaxSize = rect.size();
        } else if (rect.size().area() < mMinSize.area()) {
            mMinSize = rect.size();
        }
    }

    /**
     * Recalculates fps and posts to UI updater.
     */
    private void recalculateFPS() {
        mCount++;
        mTime = System.currentTimeMillis() - mTime;
        mFps = 1.0 / ((double) mTime / 1000.0);
        mUpdateUIHandler.post(updateFPS);
    }


    /**
     * Reduces image resolution/size by specified scale factor.
     *
     * @param inputFrame the frame to reduce the size of.
     * @return the reduced size image.
     */
    private Mat reduceImage(Mat inputFrame) {
        Mat resizedImage = new Mat((int) Math.round(inputFrame.width() / mScale_factor),
                (int) Math.round(inputFrame.height() / mScale_factor),
                inputFrame.type());
        Imgproc.resize(inputFrame, resizedImage,
                new Size((int) Math.round(inputFrame.width() / mScale_factor),
                        (int) Math.round(inputFrame.height() / mScale_factor)));
        return resizedImage;
    }
}