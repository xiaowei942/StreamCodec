package com.powervision.video.media.codec;

import android.media.MediaCodec;

/**
 * Created by liwei on 15-7-4.
 */
public class Codec {
    public static final int CODEC_TYPE_DEFAULT = 0;
    public static final int CODEC_TYPE_MEDIACODEC = 1;
    public static final int CODEC_TYPE_OTHER = 2;

    public OnFrameProcessedListener mListener = null;

    public void initCodec(Object obj){};
    public void openCodec(){};
    public void closeCodec(){};
    public void releaseCodec(){};
    public void setOnFrameProcessedListener(OnFrameProcessedListener listener) {
        mListener = listener;
    }
}
