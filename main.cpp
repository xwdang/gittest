#include <stdio.h>
#include <libavformat/avformat.h>
#include "joinmp4.h"
#include "thumbnail.h"

int main (int argv,char** args)
{   
	int test = 1;
	if(test == 0)
	{
		const int length = 3;
    	char mp4file[length][400] = {"0.mp4","1.mp4","2.mp4"};
		char h264[length][400]  = {"d.h264","d.h264","d.h264"};
		char aac[length][400] = {"d.aac","d.aac","d.aac"};
		char * mp4 = "out.mp4";
		char * h264file = "out.h264";
    	joinmp4(mp4file,mp4,length,0);
	} 
    else if(test == 1)
    {
    	char* filename = "out.mp4";
    	int width = 640;
    	int height = 640;
    	uint8_t* buff = (uint8_t*)malloc(width * height* 4  * sizeof(uint8_t));
    	
    	get_thumb_not_alloc(filename,width,height,buff );
    }
    return 0;
}