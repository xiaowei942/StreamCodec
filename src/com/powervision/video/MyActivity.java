package com.powervision.video;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import com.powervision.video.media.codec.*;
import com.powervision.video.media.extractor.DataExtractor;
import com.powervision.video.media.extractor.ExtractorFactory;
import com.powervision.video.media.extractor.IDataExtractor;

public class MyActivity extends Activity implements OnFrameProcessedListener, SurfaceHolder.Callback
{
    final boolean HASSURFACE = true;
    final boolean WINDOWDISPLAY = false;

    public Surface mSurface = null;
    int mWidth = 1280;
    int mHeight = 720;
    ICodec codec;
    IDataExtractor extractor;
    SurfaceView sv;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        if(HASSURFACE) {
            if (!WINDOWDISPLAY) {
                setContentView(R.layout.main);
                sv = (SurfaceView) findViewById(R.id.surface_view);
                sv.getHolder().addCallback(this);
            } else {
                sv = new SurfaceView(this);
                sv.getHolder().addCallback(this);
                setContentView(sv);
            }
        } else {
            setContentView(R.layout.main);
            prepare();
        }

    }

    @Override
    public void onFrameProcessed(byte[] processedFrame) {
        Log.i("TEST", "MyActivity onFrameProcessed");
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i2, int i3) {
        if(HASSURFACE) {
            mSurface = surfaceHolder.getSurface();
        } else {
            mSurface = null;
        }

        prepare();
    }

    private void prepare() {
        //IDataExtractor extractor = ExtractorFactory.createFileDataExtractor("/storage/emulated/0/test.h264");
        extractor = ExtractorFactory.createFileDataExtractor("/mnt/sdcard/test.h264");
        extractor.openDataExtractor();
        extractor.start();

        CodecParam param = new CodecParam();
        param.extractor = extractor;
        param.surface = mSurface;
        param.width = 1280;
        param.height = 720;
        CodecParam.codecType = Codec.CODEC_TYPE_MEDIACODEC;
        codec = CodecFactory.createCodec(param);
        codec.asObject().setOnFrameProcessedListener(MyActivity.this);

        codec.asObject().initCodec(null);
        codec.asObject().openCodec();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }
}
