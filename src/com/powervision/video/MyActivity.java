package com.powervision.video;

import android.app.Activity;
import android.os.Bundle;
import com.powervision.video.media.DataExtractor;
import com.powervision.video.media.ExtractorFactory;
import com.powervision.video.media.IDataExtractor;

public class MyActivity extends Activity
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        //IDataExtractor extractor = ExtractorFactory.createFileDataExtractor("/storage/emulated/0/test.h264");
        IDataExtractor extractor = ExtractorFactory.createFileDataExtractor("/storage/sdcard0/test.h264");
        extractor.openDataExtractor();
        extractor.start();
        do {

        } while(extractor.getStatus() != DataExtractor.DATA_EXTRACTOR_STATUS_OK);
    }
}
