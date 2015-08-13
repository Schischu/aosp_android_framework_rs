package com.example.noahp.detectperf;

import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;

import org.opencv.android.Utils;
import org.opencv.core.CvException;
import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;
import org.opencv.objdetect.CascadeClassifier;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;


public class MainActivity extends ActionBarActivity {

    private static final String TAG = "FACEDETECT";
    private MatOfRect mFaceDetections;
    private Mat mInputImage;
    private ImageView mInputImageView;
    private ListView mTestListView;
    private Button mDetectButton;
    private Button mResetButton;
    private ArrayList<Test> mTests;
    private TestAdapter mAdapter;
    private CascadeClassifier mDetector;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        setupViews();
        initOpenCV();
        initTests();
        initDetector();
    }

    private void initTests() {
        mTests = new ArrayList<>();
        for(int i = 0; i < 5; i++) {
            Test test = new Test();
            test.setTestFacesFound(0);
            test.setTestId(((Integer) i).toString());
            test.setTestStatus("Not Run");
            test.setTestTime(0);
            test.setHasRun(false);
            mTests.add(test);
        }
        mTests.get(0).setImage(getResources().getDrawable(R.drawable.a));
        mTests.get(0).setTestFacesExpected(4);
        mTests.get(1).setImage(getResources().getDrawable(R.drawable.b));
        mTests.get(1).setTestFacesExpected(8);
        mTests.get(2).setImage(getResources().getDrawable(R.drawable.c));
        mTests.get(2).setTestFacesExpected(4);
        mTests.get(3).setImage(getResources().getDrawable(R.drawable.d));
        mTests.get(3).setTestFacesExpected(40);
        mTests.get(4).setImage(getResources().getDrawable(R.drawable.e));
        mTests.get(4).setTestFacesExpected(5);
        mAdapter = new TestAdapter(this,mTests);
        mTestListView.setAdapter(mAdapter);
    }

    /**
     * Fill member view variables and set initial image view.
     */
    private void setupViews() {
        mInputImageView = (ImageView) findViewById(R.id.imageView);
        mDetectButton = (Button) findViewById(R.id.detect);
        mResetButton = (Button) findViewById(R.id.reset);
        mTestListView = (ListView) findViewById(R.id.listView);
        mInputImageView.setImageResource(R.drawable.a);
    }

    /**
     * Load OpenCV Java Shared Lib JNI.
     */
    private static void initOpenCV() {
        System.loadLibrary("opencv_java");

    }

    /**
     * Handle detect/camera button clicks.
     * @param v the view clicked firing this call.
     */
    public void onClick(View v){
        if (v.equals(mDetectButton)) {
            for(Test test : mTests) {
                mInputImageView.setImageDrawable(test.getImage());
                readImage(test);
                processImage(test);
                putImage();
            }
        } else if (v.equals(mResetButton)) {
            mInputImageView.setImageResource(R.drawable.a);
            initTests();
            initDetector();
        }
    }

    /**
     * Put the final image back into the image view.
     */
    private void putImage(){
        Mat result = new Mat();
        Bitmap bmp = Bitmap.createBitmap(mInputImage.width(), mInputImage.height(),Bitmap.Config.ARGB_8888);
        try {
            Imgproc.cvtColor(mInputImage, result, Imgproc.COLOR_RGB2BGRA);
            Utils.matToBitmap(result, bmp);
        } catch (CvException e) {
            Log.d(TAG, e.getMessage());
        }
        mInputImageView.setImageBitmap(bmp);
        mAdapter = new TestAdapter(this,mTests);
        mTestListView.setAdapter(mAdapter);
    }

    /**
     * Draw rectangels around detected face on image.
     * @param test the test being run
     */
    private void processImage(Test test){

        System.out.println(String.format("Detected %s faces", mFaceDetections.toArray().length));
        test.setDetections(mFaceDetections.toArray());
        test.setTestFacesFound(mFaceDetections.toArray().length);
        test.setHasRun(true);
        if(test.getTestFacesFound() == test.getTestFacesExpected()) {
            test.setTestStatus("Passed");
        } else {
            test.setTestStatus("Failed");
        }
        for (Rect rect : mFaceDetections.toArray()) {
            Imgproc.rectangle(mInputImage, new Point(rect.x, rect.y), new Point(rect.x + rect.width, rect.y + rect.height), new Scalar(0, 255, 0),3);
            Log.d(TAG,"FACE FOUND W/ W: " + rect.width + " H: " + rect.height + " @ " +  rect.x + Math.floor(rect.width/2) + "," + rect.y + Math.floor(rect.height/2));
        }
    }

    private void initDetector() {
        mDetector = new CascadeClassifier();
        try {
            InputStream is = getResources().openRawResource(R.raw.haarcascade_frontalface_alt);
            File cascadeDir = getDir("cascade", this.MODE_PRIVATE);
            File mCascadeFile = new File(cascadeDir, "haarcascade_frontalface_alt.xml");
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
     * Process image from the imageView and detect features.
     * @param test the test being run
     */
    private void readImage(Test test) {
        mInputImage = new Mat();
        Drawable d = mInputImageView.getDrawable();
        Bitmap bmp = ((BitmapDrawable) d).getBitmap();
        Bitmap bmp_RGB_8888 = bmp.copy(Bitmap.Config.ARGB_8888, true);
        Utils.bitmapToMat(bmp_RGB_8888, mInputImage);
        Imgproc.cvtColor(mInputImage, mInputImage, Imgproc.COLOR_RGB2BGRA);
        mFaceDetections = new MatOfRect();
        long start = System.nanoTime();
        mDetector.detectMultiScale(mInputImage, mFaceDetections);
        long end = System.nanoTime();
        test.setTestTime((double)(end - start)*Math.pow(10,-9));
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
}
