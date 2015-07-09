#include <stdlib.h>
#include "image_convert.h"

#include <android/log.h>
#include <android/bitmap.h>
#include <math.h>
#include <libyuv/convert_from.h>
#include <jpeglib.h>
#define LOG_TAG "Spore.meitu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

int min(int x, int y) {
    return (x <= y) ? x : y;
}
int max(int x,int y){
	return (x >= y) ? x : y;
}
int alpha(int color) {
    return (color >> 24) & 0xFF;
}
int red(int color) {
    return (color >> 16) & 0xFF;
}
int green(int color) {
    return (color >> 8) & 0xFF;
}
int blue(int color) {
    return color & 0xFF;
}
int ARGB(int alpha, int red, int green, int blue) {
    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
inline static unsigned short int make16color(unsigned char r, unsigned char g, unsigned char b)
{
    return (
 (((r >> 3) & 31) << 11) |
 (((g >> 2) & 63) << 5)  |
  ((b >> 3) & 31)        );
}

int framebuffer_main()
{
	LOGI("framebuffer code");
	int fbfd = 0;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long int screensize = 0;
	char *fbp = 0;
	int x = 0, y = 0;
	int guage_height = 20, step = 10;
	long int location = 0;
	// Open the file for reading and writing
	LOGI("framebuffer code 1");
	fbfd = open("/dev/graphics/fb0", O_RDWR);
	LOGI("framebuffer code 2");
	if (!fbfd) {
		LOGI("Error: cannot open framebuffer device.\n");
		exit(1);
	}
	LOGI("The framebuffer device was opened successfully.\n");
	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
		LOGI("Error reading fixed information.\n");
		exit(2);
	}
	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
		LOGI("Error reading variable information.\n");
		exit(3);
	}
	LOGI("sizeof(unsigned short) = %d\n", sizeof(unsigned short));
	LOGI("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
	LOGI("xoffset:%d, yoffset:%d, line_length: %d\n", vinfo.xoffset,
			vinfo.yoffset, finfo.line_length);
	// Figure out the size of the screen in bytes
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
	// Map the device to memory
	fbp = (char *) mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
			fbfd, 0);
	if ((int) fbp == -1) {
		LOGI("Error: failed to map framebuffer device to memory.\n");
		exit(4);
	}
	LOGI("The framebuffer device was mapped to memory successfully.\n");
	//set to black color first
	memset(fbp, 0, screensize);
	//draw rectangle
	y = (vinfo.yres - guage_height) / 2 - 2; // Where we are going to put the pixel
	for (x = step - 2; x < vinfo.xres - step + 2; x++) {
		location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y
				+ vinfo.yoffset) * finfo.line_length;
		*((unsigned short int*) (fbp + location)) = 255;
	}
	y = (vinfo.yres + guage_height) / 2 + 2; // Where we are going to put the pixel
	for (x = step - 2; x < vinfo.xres - step + 2; x++) {
		location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y
				+ vinfo.yoffset) * finfo.line_length;
		*((unsigned short int*) (fbp + location)) = 255;
	}
	x = step - 2;
	for (y = (vinfo.yres - guage_height) / 2 - 2; y < (vinfo.yres
			+ guage_height) / 2 + 2; y++) {
		location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y
				+ vinfo.yoffset) * finfo.line_length;
		*((unsigned short int*) (fbp + location)) = 255;
	}
	x = vinfo.xres - step + 2;
	for (y = (vinfo.yres - guage_height) / 2 - 2; y < (vinfo.yres
			+ guage_height) / 2 + 2; y++) {
		location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y
				+ vinfo.yoffset) * finfo.line_length;
		*((unsigned short int*) (fbp + location)) = 255;
	}
	// Figure out where in memory to put the pixel
	for (x = step; x < vinfo.xres - step; x++) {
		for (y = (vinfo.yres - guage_height) / 2; y < (vinfo.yres
				+ guage_height) / 2; y++) {
			location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y
					+ vinfo.yoffset) * finfo.line_length;
			if (vinfo.bits_per_pixel == 32) {
				*(fbp + location) = 100; // Some blue
				*(fbp + location + 1) = 15 + (x - 100) / 2; // A little green
				*(fbp + location + 2) = 200 - (y - 100) / 5; // A lot of red
				*(fbp + location + 3) = 0; // No transparency
			} else { //assume 16bpp
				unsigned char b = 255 * x / (vinfo.xres - step);
				unsigned char g = 255; // (x - 100)/6 A little green
				unsigned char r = 255; // A lot of red
				unsigned short int t = make16color(r, g, b);
				*((unsigned short int*) (fbp + location)) = t;
			}
		}
		//printf("x = %d, temp = %d\n", x, temp);
		//sleep to see it
		usleep(200);
	}
	//clean framebuffer
	munmap(fbp, screensize);
	close(fbfd);
	return 0;
}

int r_v_table[256],g_v_table[256],g_u_table[256],b_u_table[256],y_table[256];
int r_yv_table[256][256],b_yu_table[256][256];
int inited = 0;

void initTable()
{
	if (inited == 0)
	{
		//framebuffer_main();
		inited = 1;
		int m = 0,n=0;
		for (; m < 256; m++)
		{
			r_v_table[m] = 1634 * (m - 128);
			g_v_table[m] = 833 * (m - 128);
			g_u_table[m] = 400 * (m - 128);
			b_u_table[m] = 2066 * (m - 128);
			y_table[m] = 1192 * (m - 16);
		}
		int temp = 0;
		for (m = 0; m < 256; m++)
			for (n = 0; n < 256; n++)
			{
				temp = 1192 * (m - 16) + 1634 * (n - 128);
				if (temp < 0) temp = 0; else if (temp > 262143) temp = 262143;
				r_yv_table[m][n] = temp;

				temp = 1192 * (m - 16) + 2066 * (n - 128);
				if (temp < 0) temp = 0; else if (temp > 262143) temp = 262143;
				b_yu_table[m][n] = temp;
			}
	}
}

	jint Java_com_powervision_video_media_codec_StreamCodec_decodeYUV420SP(JNIEnv * env,
			jobject thiz, jintArray dat, jbyteArray buf, jint width, jint height) {
	jbyte * yuv420sp = (*env)->GetByteArrayElements(env, buf, 0);
	jint * rgb = (*env)->GetIntArrayElements(env, dat, 0);

	int frameSize = width * height;
	//jint rgb[frameSize]; // 新图像像素值

	initTable();

	int i = 0, j = 0,yp = 0;
	int uvp = 0, u = 0, v = 0;
	for (j = 0, yp = 0; j < height; j++)
	{
		uvp = frameSize + (j >> 1) * width;
		u = 0;
		v = 0;
		for (i = 0; i < width; i++, yp++)
		{
			int y = (0xff & ((int) yuv420sp[yp]));
			if (y < 0)
				y = 0;
			if ((i & 1) == 0)
			{
				v = (0xff & yuv420sp[uvp++]);
				u = (0xff & yuv420sp[uvp++]);
			}

//			int y1192 = 1192 * y;
//			int r = (y1192 + 1634 * v);
//			int g = (y1192 - 833 * v - 400 * u);
//			int b = (y1192 + 2066 * u);
			int y1192 = y_table[y];
			int r = r_yv_table[y][v];//(y1192 + r_v_table[v]);
			int g = (y1192 - g_v_table[v] - g_u_table[u]);
			int b = b_yu_table[y][u];//(y1192 + b_u_table[u]);

			//if (r < 0) r = 0; else if (r > 262143) r = 262143;
			if (g < 0) g = 0; else if (g > 262143) g = 262143;
			//if (b < 0) b = 0; else if (b > 262143) b = 262143;
//			r = (r >> 31) ? 0 : (r & 0x3ffff);
//			g = (g >> 31) ? 0 : (g & 0x3ffff);
//			b = (b >> 31) ? 0 : (b & 0x3ffff);

			rgb[yp] = 0xff000000 | ((r << 6) & 0xff0000) | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
		}
	}

	//jintArray result = (*env)->NewIntArray(env, frameSize);
	//(*env)->SetIntArrayRegion(env, result, 0, frameSize, rgb);
	(*env)->ReleaseByteArrayElements(env, buf, yuv420sp, 0);
	(*env)->ReleaseIntArrayElements(env, dat, rgb, 0);
	return 0;
}

jint Java_com_powervision_video_media_codec_StreamCodec_decodeYUV420SP2(JNIEnv * env,
		jobject thiz, jintArray dat, jbyteArray buf, jint width, jint height) 
{
	jbyte * yuv420sp = (*env)->GetByteArrayElements(env, buf, 0);
	jint * rgb = (*env)->GetIntArrayElements(env, dat, 0);
 
	int frameSize = width * height;
	//jint rgb[frameSize]; // 新图像像素值
 
	int i = 0, j = 0,yp = 0;
	int uvp = 0, u = 0, v = 0;
	for (j = 0, yp = 0; j < height; j++)
	{
		uvp = frameSize + (j >> 1) * width;
		u = 0;
		v = 0;
		for (i = 0; i < width; i++, yp++)
		{
			int y = (0xff & ((int) yuv420sp[yp])) - 16;
			if (y < 0)
				y = 0;
			if ((i & 1) == 0)
			{
				v = (0xff & yuv420sp[uvp++]) - 128;
				u = (0xff & yuv420sp[uvp++]) - 128;
			}
 
			int y1192 = 1192 * y;
			int r = (y1192 + 1634 * v);
			int g = (y1192 - 833 * v - 400 * u);
			int b = (y1192 + 2066 * u);
 
			if (r < 0) r = 0; else if (r > 262143) r = 262143;
			if (g < 0) g = 0; else if (g > 262143) g = 262143;
			if (b < 0) b = 0; else if (b > 262143) b = 262143;
 
			//LOGI("	 R: %d G: %d B: %d  yp: %d\n", r, g, b, yp);
			rgb[yp] = 0xff000000 | ((r << 6) & 0xff0000) | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
		}
	}
 
	//jintArray result = (*env)->NewIntArray(env, frameSize);
	//(*env)->SetIntArrayRegion(env, result, 0, frameSize, rgb);
	(*env)->ReleaseByteArrayElements(env, buf, yuv420sp, 0);
	(*env)->ReleaseIntArrayElements(env, dat, rgb, 0);
	return 0;
}


void ccvt_420p_rgb565(int width, int height, const unsigned char *src, __u16 *dst)
{
int line, col, linewidth;
int y, u, v, yy, vr, ug, vg, ub;
int r, g, b;
const unsigned char *py, *pu, *pv;

linewidth = width >> 1;
py = src;
pu = py + (width * height);
pv = pu + (width * height) / 4;

y = *py++;
yy = y << 8;
u = *pu - 128;
ug =   88 * u;
ub = 454 * u;
v = *pv - 128;
vg = 183 * v;
vr = 359 * v;

for (line = 0; line < height; line++) {
   for (col = 0; col < width; col++) {
    r = (yy +      vr) >> 8;
    g = (yy - ug - vg) >> 8;
    b = (yy + ub     ) >> 8;

    if (r < 0)   r = 0;
    if (r > 255) r = 255;
    if (g < 0)   g = 0;
    if (g > 255) g = 255;
    if (b < 0)   b = 0;
    if (b > 255) b = 255;
   *dst++ = (((__u16)r>>3)<<11) | (((__u16)g>>2)<<5) | (((__u16)b>>3)<<0);
  
    y = *py++;
    yy = y << 8;
    if (col & 1) {
     pu++;
     pv++;

     u = *pu - 128;
     ug =   88 * u;
     ub = 454 * u;
     v = *pv - 128;
     vg = 183 * v;
     vr = 359 * v;
    }
   }
   if ((line & 1) == 0) { // even line: rewind
    pu -= linewidth;
    pv -= linewidth;
   }
}
}




//*************************************************************
int convertYUVtoARGB(int y, int v, int u) {
    jint r,g,b;
 
    r = y + (int)1.402f*u;
    g = y - (int)(0.344f*v +0.714f*u);
    b = y + (int)1.772f*v;
    r = r>255? 255 : r<0 ? 0 : r;
    g = g>255? 255 : g<0 ? 0 : g;
    b = b>255? 255 : b<0 ? 0 : b;
    return 0xff000000 | (r<<16) | (g<<8) | b;
}


jint Java_com_powervision_video_media_codec_StreamCodec_YUV420PtoARGB8888(JNIEnv * env,
		jobject thiz, jintArray dat, jbyteArray buf, jint width, jint height) {
    jbyte * data = (*env)->GetByteArrayElements(env, buf, 0);
    jint * pixels = (*env)->GetIntArrayElements(env, dat, 0);
    int size = width*height;
    int offset = size;
    //int pixels[size];
    int u, v, y1, y2, y3, y4;
    // i along Y and the final pixels
    // k along pixels U and V
    int i=0, k=0;
#if 1
    for(i=0, k=0; i < size; i+=2, k+=2) {
        y1 = data[i  ]&0xff;
        y2 = data[i+1]&0xff;
        y3 = data[width+i  ]&0xff;
        y4 = data[width+i+1]&0xff;
 
        u = data[offset+k  ]&0xff;
        v = data[offset+k+1]&0xff;
        u = u-128;
        v = v-128;
 
        pixels[i  ] = convertYUVtoARGB(y1, u, v);
        pixels[i+1] = convertYUVtoARGB(y2, u, v);
        pixels[width+i  ] = convertYUVtoARGB(y3, u, v);
        pixels[width+i+1] = convertYUVtoARGB(y4, u, v);
 
        if (i!=0 && (i+2)%width==0)
            i+=width;
    }
#endif
    //jintArray result = (*env)->NewIntArray(env, size);
    //(*env)->SetIntArrayRegion(env, result, 0, size, pixels);
    (*env)->ReleaseByteArrayElements(env, buf, data, 0);
    (*env)->ReleaseIntArrayElements(env, dat, pixels, 0);
    return 123;
}





jint Java_com_powervision_video_media_codec_StreamCodec_decodeYUV420P(JNIEnv * env,
		jobject thiz, jbyteArray dat, jbyteArray buf, jint width, jint height) {
	jbyte * yuv420p = (*env)->GetByteArrayElements(env, buf, 0);
	jbyte * rgb = (*env)->GetByteArrayElements(env, dat, 0);

	I420ToARGB((unsigned char *)yuv420p, width, 
		   (unsigned char *)(yuv420p+width*height*5/4), width/2,
		   (unsigned char *)(yuv420p+width*height), width/2,
		   rgb, width*4,
		   width, height
		  );

	(*env)->ReleaseByteArrayElements(env, buf, yuv420p, 0);
	(*env)->ReleaseByteArrayElements(env, dat, rgb, 0);
}


jint Java_com_powervision_video_media_codec_StreamCodec_writeJpegFileFromYUV420(JNIEnv *env,
		jobject thiz, jstring file_name, jbyteArray yuv_data, jint quality, jint image_width, jint image_height) {
LOGI("WEI-->Into");
	jbyte *yuv = (*env)->GetByteArrayElements(env, yuv_data, 0);
	char *filename = (*env)->GetStringUTFChars(env, file_name, 0);
	if(filename == NULL) {
		return -1;
	}

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE * outfile;    // target file 
	JSAMPROW row_pointer[1];  // pointer to JSAMPLE row[s] 
	int row_stride;    // physical row width in image buffer
	JSAMPIMAGE  buffer;

	unsigned char *pSrc,*pDst;
	int band,i,buf_width[3],buf_height[3];

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_compress(&cinfo);

LOGI("WEI-->Out");
	return 0;
	if ((outfile = fopen(filename, "wb")) == NULL) {
		LOGI("WEI-->Cannot Open File");
		fprintf(stderr, "can't open %sn", filename);
		exit(1);
	}

	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = image_width;  // image width and height, in pixels
	cinfo.image_height = image_height;
	cinfo.input_components = 3;    // # of color components per pixel
	cinfo.in_color_space = JCS_RGB;  //colorspace of input image

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE );

	//////////////////////////////
	cinfo.raw_data_in = TRUE;
	cinfo.jpeg_color_space = JCS_YCbCr;
	cinfo.comp_info[0].h_samp_factor = 2;
	cinfo.comp_info[0].v_samp_factor = 2;
	//////////////////////////////
	jpeg_start_compress(&cinfo, TRUE);

	buffer = (JSAMPIMAGE) (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo,
				JPOOL_IMAGE, 3 * sizeof(JSAMPARRAY)); 
	for(band=0; band <3; band++)
	{
		buf_width[band] = cinfo.comp_info[band].width_in_blocks * DCTSIZE;
		buf_height[band] = cinfo.comp_info[band].v_samp_factor * DCTSIZE;
		buffer[band] = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo,
				JPOOL_IMAGE, buf_width[band], buf_height[band]);
	} 

	unsigned char *rawData[3];
	rawData[0]=yuv;
	rawData[1]=yuv+image_width*image_height;
	rawData[2]=yuv+image_width*image_height*5/4;

	int src_width[3],src_height[3];
	for(i=0;i<3;i++)
	{
		src_width[i]=(i==0)?image_width:image_width/2;
		src_height[i]=(i==0)?image_height:image_height/2;
	}

	int counter;
	//max_line一般为16，外循环每次处理16行数据。
	int max_line = cinfo.max_v_samp_factor*DCTSIZE; 
	for(counter=0; cinfo.next_scanline < cinfo.image_height; counter++)
	{   
		//buffer image copy.
		for(band=0; band <3; band++)  //每个分量分别处理
		{
			int mem_size = src_width[band];//buf_width[band];
			pDst = (unsigned char *) buffer[band][0];
			pSrc = (unsigned char *) rawData[band] + counter*buf_height[band] * src_width[band];//buf_width[band];  //yuv.data[band]分别表示YUV起始地址

			for(i=0; i <buf_height[band]; i++)  //处理每行数据
			{
				memcpy(pDst, pSrc, mem_size);
				pSrc += src_width[band];//buf_width[band];
				pDst += buf_width[band];
			}
		}

		jpeg_write_raw_data(&cinfo, buffer, max_line);
	}

	jpeg_finish_compress(&cinfo);
	fclose(outfile);
	jpeg_destroy_compress(&cinfo);

	(*env)->ReleaseStringUTFChars(env, file_name, filename);
	(*env)->ReleaseByteArrayElements(env, yuv_data, yuv, 0);

LOGI("WEI-->Out");
	return 0;
}
