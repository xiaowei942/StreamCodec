package com.powervision.video.media.codec;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Environment;
import android.text.format.Time;
import android.util.Log;
import android.view.Surface;
import com.powervision.video.media.extractor.FileDataExtractor;
import com.powervision.video.media.extractor.IDataExtractor;
import com.powervision.video.writer.AVCWriter;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.List;

/**
 * Created by liwei on 15-7-4.
 */
public class StreamCodec extends Codec implements ICodec {
    private static final String TAG = "StreamCodec";
    MediaCodec codec = null;
    // parameters for the decoder
    private static final String MIME_TYPE = "video/avc";    // H.264 Advanced Video Coding
    private static final String DEBUG_OUT_FILE_NAME_BASE = "/sdcard/my_decoded.";
    private static final String DEBUG_OUT_FILE_NAME_BASE_IN = "/sdcard/my_encoded.";

    private static final long TIMEOUT_USEC = 10000;
    private static final boolean VERBOSE = true;
    private static final boolean OUTPUTBUFFERS = false;
    private static final boolean DEBUG_SAVE_FILE = true;
    private static final boolean OUTPUT_YUV_TO_FILE = false;
    private static final boolean OUTPUT_RGB_TO_FILE = false;

    private static byte[] brgb = null;
    private static int[] irgb = null;

    static Bitmap surfaceBitmap = null;
    static Bitmap captureBitmap = null;
    public static boolean framePrepared = false;
    private boolean captureFrame = false;

    long rawSize = 0;
    int decodedframes = 0;

    // size of a frame, in pixels
    static int mWidth = -1;
    static int mHeight = -1;
    static Surface mSurface = null;
    static boolean hasSurface = false;
    static IDataExtractor mExtractor = null;

    FileOutputStream outputStream = null;
    FileOutputStream outputStream_out_yuv = null;
    FileOutputStream outputStream_out_rgb = null;

    ByteBuffer[] decoderInputBuffers = null;
    ByteBuffer[] decoderOutputBuffers = null;
    MediaFormat decoderOutputFormat = null;

    boolean decoderconfigured = false;

    // Loop until the output side is done.
    boolean inputDone = false;
    boolean encoderDone = false;
    boolean outputDone = false;

    MediaCodec.BufferInfo info;

    AVCWriter writer = null;
    static boolean closeWriter = false;
    Object obj;

    public static Bitmap getFrameBitmap() {
        return surfaceBitmap;
    }

    StreamCodec(CodecParam param) {
        obj = param.obj;
        mExtractor = param.extractor;
        mSurface = param.surface;
        if(mSurface == null) {
            hasSurface = false;
        } else {
            hasSurface = true;
        }

        if ((param.width % 16) != 0 || ((param.height % 16) != 0)) {
            Log.w(TAG, "WARNING: width or height not multiple of 16");
        }
        mWidth = param.width;
        mHeight = param.height;

        irgb = new int[mWidth * mHeight];

        if (brgb == null) {
            brgb = new byte[mWidth * mHeight * 4];
        }
        // Initialize the bitmap, with the replaced color
        surfaceBitmap = Bitmap.createBitmap(mWidth, mHeight,
                Bitmap.Config.ARGB_8888);
        /* We cannot use following way to create a bitmap, or error occurs when setPixels
            surfaceBitmap = Bitmap.createBitmap(irgb, mWidth, mHeight,
                    Bitmap.Config.ARGB_8888);
        */

        writer = new AVCWriter(mWidth, mHeight, 30);
        writer.open();
    }

    public static void setCloseWriter(boolean close) {
        closeWriter = close;
    }

    public boolean getCaptureFrame() {
        return captureFrame;
    }

    public void setCaptureFrame(boolean isCap) {
        captureFrame = isCap;
    }

    void initMediaCodec(ByteBuffer sps, int spsLength, ByteBuffer pps, int ppsLength) {
//
        byte[] spsBuf = new byte[spsLength];
        sps.get(spsBuf, 0, spsLength);
        sps.position(0);
        byte[] ppsBuf = new byte[ppsLength];
        pps.get(ppsBuf, 0, ppsLength);
        pps.position(0);
        writer.writeFrame(spsBuf, spsLength, 1);
        writer.writeFrame(ppsBuf, ppsLength, 1);
        info = new MediaCodec.BufferInfo();
        codec = MediaCodec.createDecoderByType(MIME_TYPE);
        MediaFormat format = MediaFormat.createVideoFormat(MIME_TYPE, mWidth, mHeight);

        format.setByteBuffer("csd-0", sps);
        format.setByteBuffer("csd-1", pps);
        //format.setInteger("color-format", 19);
        codec.configure(format, mSurface, null, 0);
        codec.start();

        decoderconfigured = true;
        decoderInputBuffers = codec.getInputBuffers();
        decoderOutputBuffers = codec.getOutputBuffers();
    }

    @Override
    public void initCodec(Object obj) {
        super.initCodec(obj);
        initMediaCodec(mExtractor.getSps(), ((FileDataExtractor)mExtractor).getSpsLength(), mExtractor.getPps(), ((FileDataExtractor)mExtractor).getPpsLength());
    }

    @Override
    public void openCodec() {
        super.openCodec();
        Decode decode = new Decode();
        new Thread(decode).start();
    }

    @Override
    public void closeCodec() {
        super.closeCodec();
    }

    @Override
    public void releaseCodec() {
        super.releaseCodec();
    }

    @Override
    public Codec asObject() {
        return (Codec) this;
    }

    @Override
    public ICodec asInterface() {
        return (ICodec)this;
    }

    @Override
    public void processFrame(byte[] frame) {

        if (mListener != null) {
            byte[] ret = new byte[0];
            mListener.onFrameProcessed(ret);
        }
    }

    private String makeCaptureFileName() {
        Time time = new Time();
        time.setToNow();
        //return new String(Environment.getExternalStorageDirectory() + "/" + time.year + "-" + time.month + "-" + time.monthDay + "-" + time.hour + "-" + time.minute + "-" + time.second + ".jpg");
        return new String("/sdcard" + "/" + time.year + "-" + time.month + "-" + time.monthDay + "-" + time.hour + "-" + time.minute + "-" + time.second + ".jpg");
    }

    public class Capture implements Runnable {
        @Override
        public void run() {
            do {
                try {
                    String name = makeCaptureFileName();
                    saveBitmapToFile(name, captureBitmap, 80);
                    Thread.sleep(100);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }while(false);
        }
    }

    public class Decode implements Runnable {
        @Override
        public void run() {

            // Save a copy to disk.
            if (!hasSurface) {
                if (OUTPUT_YUV_TO_FILE) {
                    String fileName_out = DEBUG_OUT_FILE_NAME_BASE + mWidth + "x" + mHeight + ".yuv";
                    try {
                        outputStream_out_yuv = new FileOutputStream(fileName_out);
                        Log.d(TAG, "decoded output will be saved as " + fileName_out);
                    } catch (IOException ioe) {
                        Log.w(TAG, "Unable to create debug decode output file " + fileName_out);
                        throw new RuntimeException(ioe);
                    }
                } else if(OUTPUT_RGB_TO_FILE) {
                    String fileName_out = DEBUG_OUT_FILE_NAME_BASE + mWidth + "x" + mHeight + ".rgb";
                    try {
                        outputStream_out_rgb = new FileOutputStream(fileName_out);
                        Log.d(TAG, "decoded output will be saved as " + fileName_out);
                    } catch (IOException ioe) {
                        Log.w(TAG, "Unable to create debug decode output file " + fileName_out);
                        throw new RuntimeException(ioe);
                    }
                }
            }


            //Merge SPS and PPS to one buffer by set index
            int count = 1;

            long startMs = System.currentTimeMillis();

            while (!outputDone) {
                if (!inputDone) {
                    int size = 0;
                    int inputBufferIndex = 0;
                    byte[] buf = null;
                    if ((buf = mExtractor.getFrame()) != null) {
                        inputBufferIndex = codec.dequeueInputBuffer(-1);
                        if (inputBufferIndex >= 0) {
                            ByteBuffer bf = ByteBuffer.wrap(buf, 0, buf.length);

                            bf.position(0);
                            bf.limit(buf.length);

                            if (OUTPUTBUFFERS) {
                                byte[] temp = new byte[10240];
                                bf.get(temp, 0, buf.length);
                                System.out.println("Encoded buffer: " + count);
                                System.out.println("------");
                                for (int i = 0; i < buf.length; i++)
                                    System.out.print("0x" + Integer.toHexString(temp[i]) + "  \n");
                                System.out.println("------");
                            }

                            ByteBuffer inputBuffer = decoderInputBuffers[inputBufferIndex];
                            inputBuffer.clear();
                            inputBuffer.put(buf);
                            if (count == 3) {
                                codec.queueInputBuffer(inputBufferIndex, 0, buf.length, count * 1000, 0);
                                if (VERBOSE) Log.d(TAG, "passed " + size + " bytes to decoder" + " with flags - " + 1);
                            } else {
                                codec.queueInputBuffer(inputBufferIndex, 0, buf.length, count * 1000, 0);
                                if (VERBOSE) Log.d(TAG, "passed " + size + " bytes to decoder" + " with flags - " + 0);
                            }

                            if(closeWriter) {
                                writer.close();
                            } else {
                                writer.writeFrame(buf, buf.length, 1);
                            }
                            count++;
                        }
                    } else {

                        if(closeWriter) {
                            writer.close();
                        }

                        inputDone = true;
                        inputBufferIndex = codec.dequeueInputBuffer(TIMEOUT_USEC);
                        if (inputBufferIndex >= 0) {
                            codec.queueInputBuffer(inputBufferIndex, 0, size, count * 1000, MediaCodec.BUFFER_FLAG_END_OF_STREAM);

                            if (VERBOSE)
                                Log.d(TAG, "passed " + size + " bytes to decoder" + " with flags - " + MediaCodec.BUFFER_FLAG_END_OF_STREAM
                                        + (encoderDone ? " (EOS)" : ""));
                        }
                    }
                }

                if (decoderconfigured) {
                    int decoderStatus = codec.dequeueOutputBuffer(info, TIMEOUT_USEC);
                    if (decoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {
                        // no output available yet
                        if (VERBOSE)
                            Log.d(TAG, "no output from decoder available");
                    } else if (decoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                        // The storage associated with the direct ByteBuffer may already be unmapped,
                        // so attempting to access data through the old output buffer array could
                        // lead to a native crash.
                        if (VERBOSE)
                            Log.d(TAG, "decoder output buffers changed");
                        decoderOutputBuffers = codec.getOutputBuffers();
                    } else if (decoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                        // this happens before the first frame is returned
                        decoderOutputFormat = codec.getOutputFormat();
                        if (VERBOSE)
                            Log.d(TAG, "decoder output format changed: " + decoderOutputFormat);
                    } else if (decoderStatus < 0) {
                        //fail("unexpected result from deocder.dequeueOutputBuffer: " + decoderStatus);
                    } else {  // decoderStatus >= 0
                        ByteBuffer outputFrame = decoderOutputBuffers[decoderStatus];

                        if (!hasSurface) {
                            //outputFrame.position(info.offset);
                            //outputFrame.limit(info.offset + info.size);
                            rawSize += info.size;

                            if (OUTPUT_YUV_TO_FILE) {
                                if (outputStream_out_yuv != null) {
                                    byte[] data = new byte[info.size];
                                    outputFrame.get(data);
                                    outputFrame.position(info.offset);
                                    try {
                                        outputStream_out_yuv.write(data);
                                    } catch (IOException ioe) {
                                        Log.w(TAG, "failed writing debug data to file");
                                        throw new RuntimeException(ioe);
                                    }
                                    Log.w(TAG, "successful writing debug data to file");
                                }
                            } else {
                                if (OUTPUT_RGB_TO_FILE && (outputStream_out_rgb != null)) {
                                    byte[] data = new byte[info.size];
                                    outputFrame.get(data);
                                    outputFrame.position(info.offset);

                                    try {
//                                        for(int i=0; i<irgb.length; i++) {
//                                            byte[] tmp = new byte[3];
//                                            //tmp[0] = (byte)((irgb[i]>>24) & 0xff);
//                                            tmp[0] = (byte)((irgb[i]>>16) & 0xff);
//                                            tmp[1] = (byte)((irgb[i]>>8) & 0xff);
//                                            tmp[2] = (byte)( irgb[i] & 0xff);
//                                            outputStream_out_rgb.write(tmp);
//                                        }
//                                        outputStream_out_rgb.close();

                                    } catch (Exception ioe) {
                                        Log.w(TAG, "failed writing debug data to file");
                                        throw new RuntimeException(ioe);
                                    }
                                    Log.w(TAG, "successful writing debug data to file");
                                } else {

                                    byte[] data = new byte[info.size];
                                    outputFrame.get(data);
                                    outputFrame.position(info.offset);

                                    while (true) {
                                        if (!framePrepared) {
//                                                YUV420PtoARGB8888(irgb, data, mWidth, mHeight);
//                                                surfaceBitmap.setPixels(irgb, 0, mWidth, 0, 0, mWidth, mHeight);
                                                decodeYUV420P(brgb, data, mWidth, mHeight);
                                                ByteBuffer byteBuffer = ByteBuffer.wrap(brgb);
                                                surfaceBitmap.copyPixelsFromBuffer(byteBuffer);
                                                Log.i(TAG, "Decoded !!!");

                                            if (getCaptureFrame()) {
                                                captureBitmap = Bitmap.createBitmap(surfaceBitmap);
                                            }
                                            framePrepared = true;

                                            if (getCaptureFrame()) {
                                                setCaptureFrame(false);
                                                Capture cap = new Capture();
                                                new Thread(cap).start();
                                            }
                                            break;
                                        } else {
                                            try {
                                                Thread.sleep(10);
                                                break;
                                            } catch (InterruptedException e) {
                                                e.printStackTrace();
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if (info.size == 0) {
                            if (VERBOSE) Log.d(TAG, "got empty frame");
                        } else {
                            if (VERBOSE) Log.d(TAG, "decoded, checking frame " + decodedframes++);
                        }

                        if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                            if (VERBOSE) Log.d(TAG, "output EOS");
                            outputDone = true;
                        }

                        if (!hasSurface) {
                            codec.releaseOutputBuffer(decoderStatus, false /*render*/);
                        } else {
                            codec.releaseOutputBuffer(decoderStatus, true /*render*/);
                        }
                    }
                }
            }
        }
    }

    private void saveBitmapToFile(String bitName, Bitmap bitmap, int quality) throws IOException {
        File f = new File(bitName);
        f.createNewFile();
        FileOutputStream fOut = null;

        try {
            fOut = new FileOutputStream(f);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        bitmap.compress(Bitmap.CompressFormat.JPEG, quality, fOut);
        try {
            fOut.flush();
            fOut.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    static {
        System.loadLibrary("yuv");
        System.loadLibrary("yuv2rgb");
        System.loadLibrary("mp4v2");
        System.loadLibrary("video");
    }

    private native int decodeYUV420P(byte[] data, byte[] yuv420, int width, int height);
    private native int decodeYUV420SP(byte[] data, byte[] yuv420, int width, int height);
    private native int decodeYUV420SP(int[] data, byte[] yuv420, int width, int height);
    private native int decodeYUV420SP2(int[] data, byte[] yuv420, int width, int height);
    private native int YUV420PtoARGB8888(int[] data, byte[] buf, int width, int height);
}