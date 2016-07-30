#ifndef _JOINMP4_H_
#define _JOINMP4_H_

#include <stdio.h>
#include <libavformat/avformat.h>
	
  
//链接h264流
int joinmp4(char (*mp4files)[400] ,char * mp4,int length,int usefilter);
#endif