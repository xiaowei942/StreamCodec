package com.powervision.video.media.codec;

import android.view.Surface;
import com.powervision.video.media.extractor.IDataExtractor;

/**
 * Created by liwei on 15-7-6.
 */
public class CodecParam {
    public IDataExtractor extractor = null;
    public static int codecType = Codec.CODEC_TYPE_DEFAULT;
    public Surface surface = null;
    public int width = -1;
    public int height = -1;
}
