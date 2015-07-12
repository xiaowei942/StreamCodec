#include <jni.h>
#include <time.h>
#include <mp4v2/mp4v2.h>
#include <config.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <arpa/inet.h>
#include <android/log.h>

#define LOG_TAG "MP4_WRITER"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using namespace std;

typedef unsigned char byte;
#define MAX_GROUP 2
#define MAX_VENC 2
#define H264_IFRAME 1
#define H264_SPSPPS 1

bool is_nalu(byte * data, size_t size, int& type) {
	if(size < 4)return false;

		if(data[0] == 0 && data[1] == 0) {
		if(data[2] == 1){
			type = data[3] & 0x1F;
			return true;
		}
		if(data[2] == 0 && data[3] == 1) {
			if(size < 5) 
				return false;
			type = data[4] & 0x1F;
		return true;
		}
	}
	return false;
}

void deal_sps(byte * data, size_t size, vector<byte>& sps) {
	for(size_t i = 0; i < size; ++i){
		int nalu_type = -1;
		if(is_nalu(&data[i], size - i, nalu_type)) {
			if(nalu_type == 1 || nalu_type == 5 || nalu_type == 6){ // slice, sei
				return;
			}
		}
		sps.push_back(data[i]);
	}
}

int findNalPos( byte* data, size_t size, int& type )
{
	size_t total = size;
	while( !is_nalu( &data[total-size], size, type ) ){
		--size;
	}
	return total-size;
}

void make_dsi( unsigned int sampling_frequency_index, unsigned int channel_configuration, unsigned char* dsi );
int get_sr_index(unsigned int sampling_frequency);

class mp4v2_imp{
	bool   m_b_open;
	bool   video_only;
	std::string  m_str_name;
	MP4FileHandle m_file;
	MP4TrackId  m_video;
	MP4TrackId  m_audio;
public:
	mp4v2_imp():
		 video_only(true),
		 m_b_open(false),
		 m_str_name(""),
		 m_file(NULL),
		 m_video(0),
		 m_audio(0)
	{

	}

	~mp4v2_imp()
	{
		close();
	}

	int open( const char* name, int w, int h, int frame, int sampler, int pernum, unsigned char *sps, int sps_size)
	{
		//int type = 0;
		//int pos = findNalPos( &sps[8], sps_size - 8, type );
		//pos += 8;
		//int idx = 0;
		//printf( "OpenFile: name:%s, ", name );
		//for( int i = 0; i < sps_size; ++i )
		// printf( "%#x,", sps[i] );
		//printf( "\n" );
		//printf( "OpenFile: name:%s, sps_len:%d, pps_len:%d, pps[0]=%#x,pps[1]=%#x,pps[2]=%#x,pps[3]=%#x\n", name, sps_size, sps_size - pos, sps[pos + 4], sps[pos + 5], sps[pos + 6], sps[pos + 7] );
		//int sps_len = 22;
		//int pps_len = sps_size - pos - 4;
		//unsigned char sps_header[40], pps_header[40];
		//memcpy( sps_header, sps + 4, sps_len );
		//memcpy( pps_header, sps + pos + 4, pps_len );
		//printf( "pps_len:%d, pps_len:%d\n", pps_len, sps_len );
		
		LOGI("Into open");

		m_str_name = name;
		m_file = MP4CreateEx( name, 0, 1, 1, 0, 0, 0, 0);//创建mp4文件
		if ( m_file == MP4_INVALID_FILE_HANDLE)
		{
			LOGE("open file fialed.\n");
			return -1;
		}

		MP4SetTimeScale( m_file, 90000);

		//添加h264 track   
		m_video = MP4AddH264VideoTrack( m_file, 90000, 90000 / 26, w, h,
		0, //sps[5], //sps[1] AVCProfileIndication
		0, //sps[6], //sps[2] profile_compat
		0, //sps[7], //sps[3] AVCLevelIndication
		0); //3); // 4 bytes length before each NAL unit

		if ( m_video == MP4_INVALID_TRACK_ID)
		{
			LOGE("add video track failed.\n");
			return -2;
		}
		MP4SetVideoProfileLevel( m_file, 0x7f);
		
		if(!video_only) {
			//添加aac音频; //sampler与总时长成反比， pernum与总时长成正比. MP4_MPEG2_AAC_MAIN_AUDIO_TYPE这个值可以参照MP4Info例子程序的值解释。
			m_audio = MP4AddAudioTrack(m_file, 44100, 1024, MP4_MPEG2_AAC_MAIN_AUDIO_TYPE );
			if ( m_audio == MP4_INVALID_TRACK_ID)
			{
				LOGE("add audio track failed.\n");
				return -3;
			}
			MP4SetAudioProfileLevel( m_file, 0x2 );   ////0x2 ?

			//MP4AddH264SequenceParameterSet( m_file, m_video, sps_header, sps_len ); 
			//MP4AddH264PictureParameterSet( m_file, m_video, pps_header, pps_len ); 
			//
			uint8_t pConfig[2];
			uint32_t configSize = 2;
			//make_dsi( get_sr_index( 44100 ), 2, pConfig );
			if( !MP4SetTrackESConfiguration( m_file, m_audio, pConfig, configSize ) )
			{
				LOGE("MP4SetTrackESConfiguration call failed.\n");
			}

		}
		m_b_open = true;

		LOGI("Out open");
		return 0;
	}

	int close()
	{
		LOGI("Into close");
		MP4Close( m_file );
		m_file = 0;
		//MP4Optimize( m_str_name.c_str() );
	}

	////frametype: 1 keyframe, other 0; 还有这里的关键帧的参数貌似没效果，我一直都设为1，也没有问题。
	int write_video( unsigned char * data, size_t size, int frametype, unsigned long long ts )
	{
		LOGI("Now Into write_video");
		if( NULL == m_file || m_video == 0 ) 
			return -1;
		
		LOGI("data: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", data[0], data[1], data[2], data[3], data[4], data[5]);
		bool bRtn = MP4WriteSample( m_file, m_video, data, size, MP4_INVALID_DURATION, 0, 1 );
		LOGI("Now Out write_video");
		return bRtn ? 0 : -2;
	}

	int write_audio( unsigned char * data, size_t size, unsigned long long ts )
	{
		if( NULL == m_file || m_audio == 0 ) 
			return -1;

		bool bRtn = MP4WriteSample( m_file, m_audio, data, size, MP4_INVALID_DURATION, 0, 1 );
		return bRtn ? 0 : -2;
	}
};

struct video {
	int cap_width;
	int cap_height;
	int framerate;
};

struct audio {
	int samplerate;
};

struct g_group {
	struct video vi;
	struct audio ai;
}g_group[0];

class write_mp4{
	mp4v2_imp* m_mp4;
	bool m_b_loop;
	string file_path;
public:
	write_mp4(const char *path)
	: m_b_loop( false )
	{
		m_mp4 = 0;
		file_path = path;
		//memset( m_mp4, 0, sizeof(m_mp4) );
	}

	~write_mp4()
	{
		for( int i = 0; i < MAX_GROUP*MAX_VENC; ++i )
		{
			close( i );
		}
	}

	void write_video(int chn, unsigned char * data, size_t size, int frametype, unsigned long long ts )
	{
		LOGI("Into write_video");
		static time_t tt_old[MAX_GROUP*MAX_VENC] = {0};
		if( data == 0 || size == 0 ) return;
		int iRtn = 0;
		if( !m_mp4 )
		{
			if( H264_SPSPPS == frametype ) {
				std::vector< unsigned char > v;
				deal_sps( data, size, v );
				m_mp4 = new mp4v2_imp(); if( !m_mp4 ) return;

				LOGI("Now prepare to open");
				//iRtn = m_mp4[chn]->open( get_name( chn ).append( ".mp4" ).c_str(), 
				iRtn = m_mp4->open( (file_path + get_name( chn )).append( ".mp4" ).c_str(), 
							g_group[0].vi.cap_width, 
							g_group[0].vi.cap_height, 
							g_group[0].vi.framerate, 
							g_group[0].ai.samplerate, 
							1024, 
							&v[0], 
							v.size() );
				if( iRtn != 0 ) 
					return;

				LOGI("Now success open");
				tt_old[chn] = time(0);
			}
		}

		if( m_mp4 )
		{
			m_mp4->write_video( data, size, frametype == H264_IFRAME ? 1 : 0, ts );
		}

		time_t tt_new = time(0);
		if( tt_new - tt_old[chn] > 240 ) 
			close( chn );
	}

	void write_audio( int chn, unsigned char* data, size_t size, int frametype, unsigned long long ts )
	{
		if( m_mp4 )
		{
			m_mp4->write_audio( data + 7, size - 7, ts );
		}
	}

	void close( int chn )
	{
		if( m_mp4 ){
			m_mp4->close();
			delete m_mp4;
			m_mp4 = 0;
		}
	}

private:
	string get_name( int chn_id )
	{
		string strRet = "1900-01-01";
		time_t tm     = time(0);
		struct tm *newtime = localtime( ( const time_t * )( &tm ) );
		if( newtime )
		{
			char buf[128];
			sprintf( buf, "%02d-%04d%02d%02d-%02d%02d%02d", chn_id, newtime->tm_year + 1900, newtime->tm_mon+1, newtime->tm_mday,newtime->tm_hour,newtime->tm_min,newtime->tm_sec );
			strRet = buf;
		}
		return strRet;
	}
};




/**************************  Following are jni fuctions  *****************************/

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    nativeinit
 * Signature: (Ljava/lang/String;III)I
 */
write_mp4 * JNICALL Java_com_powervision_video_writer_AVCWriter_native_1init
  (JNIEnv *env, jobject thiz, jstring path, jint width, jint height, jint frame_rate) {
	LOGI("Into init with width:%d  height:%d", width, height);
	g_group[0].vi.cap_width = width;
	g_group[0].vi.cap_height = height;
	g_group[0].vi.framerate = frame_rate;
	const char *file_path = env->GetStringUTFChars(path, 0);
	write_mp4 *p = new write_mp4(file_path);
	env->ReleaseStringUTFChars(path, file_path);
	LOGI("Out init");
	if(!p) {
		return 0;
	}
	return p;
}

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_open
 * Signature: (I)I
 */
jint JNICALL Java_com_powervision_video_writer_AVCWriter_native_1open
  (JNIEnv *env, jobject thiz, write_mp4 *obj) {

}

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_write
 * Signature: (II[BJIJ)I
 */
jint JNICALL Java_com_powervision_video_writer_AVCWriter_native_1write
  (JNIEnv *env, jobject thiz, write_mp4 *obj, jint channel, jbyteArray data, jlong size, jint frame_type, jlong ts) {
	LOGI("WEI-->Into write");
	jbyte *pdata = env->GetByteArrayElements(data, 0);	  
	obj->write_video(channel, (unsigned char *)pdata, size, frame_type, ts);
	env->ReleaseByteArrayElements(data, pdata, 0);
	LOGI("WEI-->Out write");
	return 0;
}

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_close
 * Signature: (I)I
 */
jint JNICALL Java_com_powervision_video_writer_AVCWriter_native_1close
  (JNIEnv *env, jobject thiz, write_mp4 *obj) {

}

/*
 * Class:     com_powervision_video_writer_AVCWriter
 * Method:    native_release
 * Signature: (I)V
 */
void JNICALL Java_com_powervision_video_writer_AVCWriter_native_1release
  (JNIEnv *env, jobject thiz, write_mp4 *obj) {
	if(obj) {
		delete obj;
	}
}


/********************************  For Register Functions  ***********************************/
static JNINativeMethod methods[]{ 
	{"native_init", "(Ljava/lang/String;III)I", (void *)Java_com_powervision_video_writer_AVCWriter_native_1init}, 
	{"native_open", "(I)I", (void *)Java_com_powervision_video_writer_AVCWriter_native_1open}, 
	{"native_write", "(II[BJIJ)I", (void *)Java_com_powervision_video_writer_AVCWriter_native_1write}, 
	{"native_close", "(I)I", (void *)Java_com_powervision_video_writer_AVCWriter_native_1close},
	{"native_release", "(I)V", (void *)Java_com_powervision_video_writer_AVCWriter_native_1release}, 
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
 
    /* This function will be call when the library first be loaded */ 
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
