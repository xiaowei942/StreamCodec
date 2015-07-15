#include <stdio.h>
#include <iostream>
#include <mp4v2/mp4v2.h>

#include "interface.h"
#include "mp4_writer.h"
#include "mp4_extractor.h"
#include <android/log.h>
using namespace std;

#define LOG_TAG "MP4_WRITER"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define FILE_PATH "/home/liwei/Desktop/720p_1930kbs_30帧.264"
#define MP4_FILE_PATH "/home/liwei/Desktop/test.mp4"
#define MP4_FPS 30

int main() {
       	H264_Extractor *extractor = new H264_Extractor();
	extractor->get_to_list(FILE_PATH);
	extractor->get_sps_pps();

#if 0
	MP4FileHandle mp4_file = MP4CreateEx("/home/liwei/Desktop/test.mp4", 0, 1, 1, 0, 0, 0, 0);
	if(mp4_file == MP4_INVALID_FILE_HANDLE) {
		printf("Create mp4 file failed\n");
		return -1;
	}

	MP4SetTimeScale(mp4_file, 90000);
	MP4TrackId video = MP4AddH264VideoTrack(mp4_file, 90000, 90000/25, 1280, 720, extractor->sps[1], extractor->sps[2], extractor->sps[3], 3);
	if(video == MP4_INVALID_TRACK_ID) {
		printf("Create video track failed\n");
		return -1;
	}
	MP4AddH264SequenceParameterSet(mp4_file, video, extractor->sps, 26);
	MP4AddH264PictureParameterSet(mp4_file, video, extractor->pps, 5);
#endif

	Mp4_Writer *writer = new Mp4_Writer(1280, 720);
	writer->SetMp4FileName(MP4_FILE_PATH);
	writer->SetMp4Fps(MP4_FPS);
	writer->DoStartRecord();

	unsigned char *payload_data;
	unsigned int payload_size;
	unsigned int time_stamp;
	while( (payload_data = extractor->get_frame(payload_size, time_stamp)) != NULL ) {
		writer->WriteEncodedVideoFrame(payload_data, payload_size, time_stamp);
		extractor->release_frame(&payload_data);
	}
	writer->DoStopRecord();
	return 0;
}


/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_writerInit
 * Signature: (II)I
 */
JNIEXPORT Mp4_Writer * JNICALL Java_com_powervision_video_writer_AVCWriter_native_1writerInit
  (JNIEnv *env, jobject thiz, jint width, jint height) {
	Mp4_Writer *writer = new Mp4_Writer(width, height);
	if(writer) {
		return writer;
	}
	return NULL;
  }

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_setMp4FileName
 * Signature: (Ljava/lang/Object;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_powervision_video_writer_AVCWriter_native_1setMp4FileName
  (JNIEnv *env, jobject thiz, Mp4_Writer *obj, jstring file_name) {
	const char *name = env->GetStringUTFChars(file_name, 0);
	obj->SetMp4FileName(name);
	env->ReleaseStringUTFChars(file_name, name);
  }

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_setMp4Fps
 * Signature: (Ljava/lang/Object;I)V
 */
JNIEXPORT void JNICALL Java_com_powervision_video_writer_AVCWriter_native_1setMp4Fps
  (JNIEnv *env, jobject thiz, Mp4_Writer *obj, jint fps) {
	obj->SetMp4Fps(fps);
  }

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_startRecord
 * Signature: (Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_powervision_video_writer_AVCWriter_native_1startRecord
  (JNIEnv *env, jobject thiz, Mp4_Writer *obj) {
	obj->DoStartRecord();
  }

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_stopRecord
 * Signature: (Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_powervision_video_writer_AVCWriter_native_1stopRecord
  (JNIEnv *env, jobject thiz, Mp4_Writer *obj) {
	obj->DoStopRecord();
  }

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_writeFrame
 * Signature: (Ljava/lang/Object;[BJJ)V
 */
JNIEXPORT void JNICALL Java_com_powervision_video_writer_AVCWriter_native_1writeFrame
  (JNIEnv *env, jobject thiz, Mp4_Writer *obj, jbyteArray data, jlong size, jlong ts) {
	jbyte *payload_data = env->GetByteArrayElements(data, 0);
	obj->WriteEncodedVideoFrame((unsigned char *)payload_data, size, ts);
	env->ReleaseByteArrayElements(data, payload_data, 0);
  }

static JNINativeMethod methods[] = {
	{ "native_writerInit", "(II)I", (void *)Java_com_powervision_video_writer_AVCWriter_native_1writerInit },
	{ "native_setMp4FileName", "(ILjava/lang/String;)V", (void *)Java_com_powervision_video_writer_AVCWriter_native_1setMp4FileName },
	{ "native_setMp4Fps", "(II)V", (void *)Java_com_powervision_video_writer_AVCWriter_native_1setMp4Fps },
	{ "native_startRecord", "(I)V", (void *)Java_com_powervision_video_writer_AVCWriter_native_1startRecord },
	{ "native_stopRecord", "(I)V", (void *)Java_com_powervision_video_writer_AVCWriter_native_1stopRecord },
	{ "native_writeFrame", "(I[BJJ)V", (void *)Java_com_powervision_video_writer_AVCWriter_native_1writeFrame }
};

static const char * classPathName = "com/powervision/video/writer/AVCWriter";

static int registerNativeMethods(JNIEnv* env, const char* className,
        JNINativeMethod* gMethods, int numMethods) 
{ 
        jclass clazz;
        clazz = env->FindClass(className);
        if (clazz == NULL) { 
                LOGE("Native registration unable to find class '%s'", className); 
                return JNI_FALSE;
        }
        if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
                LOGE("RegisterNatives failed for '%s'", className);
                return JNI_FALSE;
        }

        return JNI_TRUE;
}

static int registerNatives(JNIEnv* env)
{
        if (!registerNativeMethods(env, classPathName,
                     methods, sizeof(methods) / sizeof(methods[0]))) {
                return JNI_FALSE;
        }

        return JNI_TRUE;
}

typedef union {
        JNIEnv* env;
        void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
        UnionJNIEnvToVoid uenv;
        JNIEnv* env = NULL;
        LOGI("JNI_OnLoad!");

        if (vm->GetEnv((void**)&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
                LOGE("ERROR: GetEnv failed");
                return -1;
        }

        env = uenv.env;;

        //jniRegisterNativeMethods(env, "whf/jnitest/Person", methods, sizeof(methods) / sizeof(methods[0])); 

        if (registerNatives(env) != JNI_TRUE) {
                LOGE("ERROR: registerNatives failed");
                return -1;
        }

        return JNI_VERSION_1_4;
}

