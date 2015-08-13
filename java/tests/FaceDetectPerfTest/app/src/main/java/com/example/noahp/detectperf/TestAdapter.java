package com.example.noahp.detectperf;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.text.DecimalFormat;
import java.util.ArrayList;

/**
 * Created by noahp on 7/15/15.
 */
public class TestAdapter extends ArrayAdapter<Test> {
    public TestAdapter(Context context, ArrayList<Test> tests) {
        super(context, 0, tests);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {

        DecimalFormat df = new DecimalFormat("0.000###");
        Test test = getItem(position);

        if (convertView == null)
            convertView = LayoutInflater.from(getContext()).inflate(R.layout.test_item, parent, false);

        TextView status = (TextView) convertView.findViewById(R.id.test_status);
        TextView id = (TextView) convertView.findViewById(R.id.test_id);
        TextView time = (TextView) convertView.findViewById(R.id.test_time);
        TextView test_faces_found = (TextView) convertView.findViewById(R.id.test_faces_found);

        status.setText(test.getTestStatus());
        id.setText("Test " + test.getTestId());
        time.setText(df.format(test.getTestTime()) + "sec");
        test_faces_found.setText("Faces Found: " + test.getTestFacesFound() + "/" + test.getTestFacesExpected());

        if (test.isHasRun()) {
            if (test.getTestFacesExpected() == test.getTestFacesFound()) {
                status.setTextColor(parent.getContext().getResources().getColor(android.R.color.holo_green_light));
            } else {
                status.setTextColor(parent.getContext().getResources().getColor(android.R.color.holo_red_light));
            }
        }

        return convertView;
    }
}
