#ifndef _THUMBNAIL_H_
#define _THUMBNAIL_H_
#include <stdio.h>

extern "C"
{

#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

int scale_not_alloc(AVFrame* inframe,AVFrame* goalframe);

int get_thumb_not_alloc(char * in_filename,int width ,int height,uint8_t * buff );

#endif
