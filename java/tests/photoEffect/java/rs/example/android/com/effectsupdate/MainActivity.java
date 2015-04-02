package rs.example.android.com.effectsupdate;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.effect.Effect;
import android.media.effect.EffectContext;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.DragEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.SeekBar;

import java.io.FileNotFoundException;
import java.io.InputStream;


public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";
    private ImageView mEffectView;
    private FilteredImageView mFilteredImageView;
    private EffectContext mEffectContext;
    private Effect mEffect;
    private int mImageWidth;
    private int mImageHeight;
    private boolean mInitialized = false;
    int mCurrentEffect;
    SeekBar slider1;
    SeekBar slider2;
    SeekBar slider3;
    float pointx;
    float pointy;
    Listener mListener = new Listener();

    class Listener implements View.OnTouchListener, View.OnDragListener {
        float down_x, down_y;

        @Override
        public boolean onDrag(View v, DragEvent event) {
            float x = event.getX();
            float y = event.getY();
            down_x = x;
            down_y = y;
            return false;
        }


        @Override
        public boolean onTouch(View v, MotionEvent event) {
            int action = event.getAction();
            Log.v(TAG, " on touch " + action);

            float x = event.getX();
            float y = event.getY();
            switch (action) {
                case MotionEvent.ACTION_DOWN:
                    down_x = x;
                    down_y = y;
                    break;
                case MotionEvent.ACTION_MOVE:
                    pointx = x;
                    pointy = y; // TODO Make this a more relitvie for drags.
                    break;
            }
            return true;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mEffectView = (ImageView) findViewById(R.id.effectsview);
        mEffectView.setOnDragListener(mListener);
        mEffectView.setOnTouchListener(mListener);
        mCurrentEffect = R.id.none;
        mFilteredImageView = new FilteredImageView(mEffectView, this);


        slider1 = (SeekBar) findViewById(R.id.slider1);
        slider2 = (SeekBar) findViewById(R.id.slider2);
        slider3 = (SeekBar) findViewById(R.id.slider3);
        SeekBar.OnSeekBarChangeListener changeListener = new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                slide();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        };
        slider1.setOnSeekBarChangeListener(changeListener);
        slider2.setOnSeekBarChangeListener(changeListener);
        slider3.setOnSeekBarChangeListener(changeListener);
        Intent intent = getIntent();

        if (intent != null) {

            String s = intent.getType();
            if (s != null && s.indexOf("image/") != -1) {
                Uri data = (Uri) intent.getParcelableExtra(Intent.EXTRA_STREAM);
                if (data != null) {
                    InputStream input = null;
                    try {
                        input = getContentResolver().openInputStream(data);
                        Bitmap bitmap = BitmapFactory.decodeStream(input);
                        mFilteredImageView.setImage(bitmap);
                        return;
                    } catch (FileNotFoundException e) {
                        e.printStackTrace();
                    }

                }
            }
        }
        Bitmap bitmap = BitmapFactory.decodeResource(getResources(),
                R.drawable.traincoming);
        mFilteredImageView.setImage(bitmap);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        int id = item.getItemId();
        setEffect(id);
        Log.v(TAG, "onOptionsItemSelected " + item.getTitle());


        return super.onOptionsItemSelected(item);
    }

    private void showSliders(boolean s1, boolean s2, boolean s3) {
        slider1.setVisibility(s1 ? View.VISIBLE : View.GONE);
        slider2.setVisibility(s2 ? View.VISIBLE : View.GONE);
        slider3.setVisibility(s3 ? View.VISIBLE : View.GONE);
    }

    void setEffect(int id) {
        mCurrentEffect = id;
        switch (id) {
            case R.id.effectContrast:
                showSliders(true, false, false);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_CONTRAST);
                break;
            case R.id.effectDistortion:
                showSliders(true, true, true);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_DISTORTION);
                break;
            case R.id.effectExposure:
                showSliders(true, false, false);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_EXPOSURE);
                break;
            case R.id.effectGrain:
                showSliders(true, false, false);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_GRAIN);
                break;
            case R.id.effectHue:
                showSliders(true, false, false);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_HUE);
                break;
            case R.id.effectInvert:
                showSliders(false, false, false);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_INVERT);
                break;
            case R.id.effectShadows:
                showSliders(true, false, false);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_SHADOW);
                break;
            case R.id.effectSaturation:
                showSliders(true, false, false);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_SATURATE);
                break;
            case R.id.effectSharpen:
                showSliders(true, false, false);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_SHARPEN);
                break;
            case R.id.effectTemperature:
                showSliders(true, false, false);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_TEMPERATURE);
                break;
            case R.id.effectVignette:
                showSliders(true, true, false);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_VIGNETTE);
                break;
            case R.id.effectDehaze:
                showSliders(true, false, false);
                mFilteredImageView.setFilterType(FilteredImageView.FILTERTYPE_DEHAZE);
                break;

        }
    }

    float mStrength = .8f;


    private void slide() {
        float v1 = slider1.getProgress() / 1000f;
        float v2 = slider2.getProgress() / 1000f;
        float v3 = slider3.getProgress() / 1000f;

        mFilteredImageView.setParameters(v1, v2, v3);

    }
}
