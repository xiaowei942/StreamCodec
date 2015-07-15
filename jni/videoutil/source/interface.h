/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_powervision_video_writer_AVCWriter */
#include "mp4_writer.h"

#ifndef _Included_com_powervision_video_writer_AVCWriter
#define _Included_com_powervision_video_writer_AVCWriter
#ifdef __cplusplus
extern "C" {
#endif
#undef com_powervision_video_writer_AVCWriter_STATUS_INIT_OK
#define com_powervision_video_writer_AVCWriter_STATUS_INIT_OK 0L
#undef com_powervision_video_writer_AVCWriter_STATUS_OPEN_OK
#define com_powervision_video_writer_AVCWriter_STATUS_OPEN_OK 1L
#undef com_powervision_video_writer_AVCWriter_STATUS_WRITE_OK
#define com_powervision_video_writer_AVCWriter_STATUS_WRITE_OK 2L
#undef com_powervision_video_writer_AVCWriter_STATUS_CLOSE_OK
#define com_powervision_video_writer_AVCWriter_STATUS_CLOSE_OK 3L
/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_writerInit
 * Signature: (II)I
 */
JNIEXPORT Mp4_Writer* JNICALL Java_com_powervision_video_writer_AVCWriter_native_1writerInit
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_setMp4FileName
 * Signature: (Ljava/lang/Object;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_powervision_video_writer_AVCWriter_native_1setMp4FileName
  (JNIEnv *, jobject, Mp4_Writer *, jstring);

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_setMp4Fps
 * Signature: (Ljava/lang/Object;I)V
 */
JNIEXPORT void JNICALL Java_com_powervision_video_writer_AVCWriter_native_1setMp4Fps
  (JNIEnv *, jobject, Mp4_Writer *, jint);

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_startRecord
 * Signature: (Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_powervision_video_writer_AVCWriter_native_1startRecord
  (JNIEnv *, jobject, Mp4_Writer *);

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_stopRecord
 * Signature: (Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_powervision_video_writer_AVCWriter_native_1stopRecord
  (JNIEnv *, jobject, Mp4_Writer *);

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_writeFrame
 * Signature: (Ljava/lang/Object;[BJJ)V
 */
JNIEXPORT void JNICALL Java_com_powervision_video_writer_AVCWriter_native_1writeFrame
  (JNIEnv *, jobject, Mp4_Writer *, jbyteArray, jlong, jlong);

#ifdef __cplusplus
}
#endif
#endif
