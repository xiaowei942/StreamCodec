package com.powervision.video.media.codec;

/**
 * Created by liwei on 15-7-4.
 */
public interface ICodec {
    public Codec asObject();
    public ICodec asInterface();
    public void processFrame(byte[] frame);
}
