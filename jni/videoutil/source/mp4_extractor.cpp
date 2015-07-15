#include <stdio.h>
#include <malloc.h>
#include <list>
#include <string.h>

#include "utils.h"
#include "mp4_extractor.h"

using namespace std;

//#define DEBUG
//#define PRINT_NALU_LIST

//#define PRINT_FRAME
#if defined(PRINT_FRAME) 
#define PRINT_FRAME_NUM 10
#endif

//#define PRINT_FRAME_NUM payload_size

#define DEFAULT_FILE_SIZE 20*1024*1024

extern unsigned char *chrs_join(const unsigned char *src1, const unsigned char *src2);
H264_Extractor::H264_Extractor():
				fd(NULL),
				file_size(0),
				frame_index(0),
				h264_data(NULL)
{

}

int H264_Extractor::get_to_list(const char *file) {
	fd = fopen(file, "r");
	if(fd == NULL) {
		printf("Error when opening file\n");
		return -1;
	}

	fseek(fd, 0, SEEK_END);
	file_size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	h264_data = (unsigned char *)malloc(file_size*sizeof(unsigned char));


	fread(h264_data, file_size, sizeof(unsigned char), fd);

	unsigned char *src = h264_data;
        for(int i=0; i<file_size; i++) {
            if( (src[i] == 0x0) && (src[i+1] == 0x0)) {
                if(src[i+2] == 0x01) {

                }else if((src[i+2] == 0x0) && (src[i+3] == 0x01)) {
                    nalu_list.push_back(i);
                    i+=3;
                }
            }
        }
        nalu_list.push_back(file_size);

#ifdef PRINT_NALU_LIST
	int count = 0;
	for(it = nalu_list.begin(); it!=nalu_list.end(); count++, ++it) {
	    printf("NALU LIST -->  Count: %d   Value: %d\n", count, *it);
	}
#endif
        return nalu_list.size();
}

int H264_Extractor::get_sps_pps() {
	int size;
        bool hasSps = false;
        bool hasPps = false;

        int index = nalu_list.at(0);
        for(int i=0; i<5; i++) {
            unsigned char temp = h264_data[index+i];
            if((temp & 0x0f) == 0x07) {
			hasSps = true;
			size = nalu_list.at(1)-nalu_list.at(0)-i;
			sps = (unsigned char *)malloc(size*sizeof(unsigned char));
			memcpy(sps, h264_data+index+i, size);

#ifdef DEBUG
			printf("Has sps\n");
			for(int k=0; k<size; k++)
				printf("0x%02x ", sps[k]);
			printf("\n");
#endif
			break;
            }
        }

        if(hasSps) {
            index = nalu_list.at(1);
            for(int j=0; j<5; j++) {
                unsigned char temp = h264_data[index+j];
                if((temp & 0x0f) == 0x08) {
			hasPps = true;
			size = nalu_list.at(2)-nalu_list.at(1)-j;
			pps = (unsigned char *)malloc(size*sizeof(unsigned char));
			memcpy(pps, h264_data+index+j, size);

#ifdef DEBUG
			printf("Has pps\n");
			for(int k=0; k<size; k++)
				printf("0x%02x ", pps[k]);
			printf("\n");
#endif
	    		break;
                }
            }
        }

	if(hasSps && hasPps) {
		sps_pps = chrs_join(sps, pps);
		return 0;
	}

	return -1;
}

unsigned char *H264_Extractor::get_frame(unsigned int &payload_size, unsigned int &time_stamp) {
	unsigned char *data;
	if(frame_index>=nalu_list.size()-1)
		return NULL;

	int start_pos = nalu_list.at(frame_index);
	int end_pos = nalu_list.at(frame_index+1);
	payload_size = end_pos - start_pos;

	data = (unsigned char *)malloc(payload_size*sizeof(unsigned char));	
	memcpy(data, (void *)h264_data+start_pos, payload_size);

#ifdef PRINT_FRAME
	printf("FRAME[%05d]--> [TS: %05d] -->  ", frame_index, time_stamp);
	for(int k=0; k<PRINT_FRAME_NUM; k++)
		printf("0x%02x ", data[k]);
	printf("\n");
#endif
	frame_index++;
	time_stamp = frame_index*3600;
	return data;
}

void H264_Extractor::release_frame(unsigned char **buf) {
	if(*buf) {
		free(*buf);
		*buf = NULL;
	}
}
