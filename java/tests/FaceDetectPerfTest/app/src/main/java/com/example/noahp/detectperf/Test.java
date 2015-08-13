package com.example.noahp.detectperf;

import android.graphics.drawable.Drawable;

import org.opencv.core.Rect;

/**
 * Created by noahp on 7/15/15.
 */
public class Test {
    public String testId;
    public String testStatus;
    public double testTime;
    public int testFacesFound;
    public Drawable image;
    public int testFacesExpected;
    public boolean hasRun;
    public Rect[] detections;

    public Rect[] getDetections() {
        return detections;
    }

    public void setDetections(Rect[] detections) {
        this.detections = detections;
    }


    public boolean isHasRun() {
        return hasRun;
    }

    public void setHasRun(boolean hasRun) {
        this.hasRun = hasRun;
    }

    public Drawable getImage() {
        return image;
    }

    public void setImage(Drawable image) {
        this.image = image;
    }

    public int getTestFacesExpected() {
        return testFacesExpected;
    }

    public void setTestFacesExpected(int testFacesExpected) {
        this.testFacesExpected = testFacesExpected;
    }

    public int getTestFacesFound() {
        return testFacesFound;
    }

    public void setTestFacesFound(int testFacesFound) {
        this.testFacesFound = testFacesFound;
    }

    public String getTestStatus() {
        return testStatus;
    }

    public void setTestStatus(String testStatus) {
        this.testStatus = testStatus;
    }

    public double getTestTime() {
        return testTime;
    }

    public void setTestTime(double testTime) {
        this.testTime = testTime;
    }

    public String getTestId() {
        return testId;
    }

    public void setTestId(String testId) {
        this.testId = testId;
    }

}
