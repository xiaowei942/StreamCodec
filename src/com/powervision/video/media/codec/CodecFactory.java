package com.powervision.video.media.codec;

import com.powervision.video.MyActivity;

/**
 * Created by liwei on 15-7-5.
 */
public class CodecFactory {
    public static ICodec createCodec(CodecParam param) {
        switch (param.codecType) {
            case Codec.CODEC_TYPE_OTHER:
                return null;
            case Codec.CODEC_TYPE_MEDIACODEC:
                return new StreamCodec(param);
            case Codec.CODEC_TYPE_DEFAULT:
                return new StreamCodec(param);
            default:
                return null;
        }
    }
}
