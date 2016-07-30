#include <stdio.h>
#include "thumbnail.h"



int scale_not_alloc(AVFrame* inframe,AVFrame* goalframe)
{
    if(inframe==NULL)
    {
        printf("[@scale]:give the frame is NULL\n");
        return -1;
    }

	struct SwsContext *img_convert_ctx;

	img_convert_ctx = sws_getContext(inframe->width, inframe->height,
			(enum AVPixelFormat )inframe->format, goalframe->width, goalframe->height, (enum AVPixelFormat )goalframe->format, SWS_BICUBIC,
			NULL, NULL, NULL);

	sws_scale(img_convert_ctx, (const uint8_t * const *) inframe->data,
			inframe->linesize, 0, inframe->height,goalframe->data,
			goalframe->linesize);
    sws_freeContext(img_convert_ctx);
	return 0;
}
//获取缩图,过滤镜
int get_thumb_not_alloc(char * in_filename,int width ,int height,uint8_t* buf )
{

	int retu = 0;
	int got_frame =0;
	AVFormatContext *ifmt_ctx = NULL;
	AVCodecContext * video_dec_ctx = NULL;
	AVCodec * video_dec = NULL;
	int i =0, videoindex = -1, ret = 0;
	AVPacket pkt;
	AVFrame* decode_frame = NULL, *scale_frame = NULL,*ret_frame = NULL;
	enum AVPixelFormat pfm;
	int h =0;
	int w = 0;
	pfm=AV_PIX_FMT_BGRA;

	av_register_all();
    decode_frame= av_frame_alloc();

     //获取各种上下文
	if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		puts("Could not open input file.");
		retu = -1;
		goto end;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		puts("Failed to retrieve input stream information");
		retu = -2;
		goto end;
    }
	//例遍流，找到流所对应得下标
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
		}
    }
	video_dec_ctx = ifmt_ctx->streams[videoindex]->codec;
	video_dec = avcodec_find_decoder(video_dec_ctx->codec_id);
	if(avcodec_open2(video_dec_ctx,video_dec,NULL)<0)
	{
		puts("Could not open decoder\n");
		goto end;
	}
 	h = ifmt_ctx->streams[videoindex]->codec->height;
	w = ifmt_ctx->streams[videoindex]->codec->width;
	if(width <= 0 )
		width = w;
	if(height <= 0)
		height = h;
    scale_frame = av_frame_alloc();
    while(av_read_frame(ifmt_ctx, &pkt) >= 0)
    {
        if (pkt.stream_index == videoindex)
		{
            ret = avcodec_decode_video2(video_dec_ctx,decode_frame, &got_frame, &pkt);
            if(got_frame)
            {
            	av_free_packet(&pkt);
            	break;
            }
            av_free_packet(&pkt);
		}
        
    }
    if(got_frame)
    {
    	ret_frame = decode_frame;

		if (width != ret_frame->width || height != ret_frame->height
				|| pfm != ret_frame->format) {
            
        	avpicture_fill((AVPicture *)scale_frame,(uint8_t *)buf,pfm,width,height);
    		scale_frame->width = width;
    		scale_frame->height = height;
    		scale_frame->format = pfm;
			ret = scale_not_alloc(ret_frame, scale_frame);
			ret_frame = scale_frame;
			uint8_t * dst_data[4];
			int dst_linesize[4];
			enum AVPixelFormat dst = AV_PIX_FMT_BGRA;
			av_image_alloc(dst_data,dst_linesize,width,height,dst,1);

			puts("s");
		}
  }
end:
	if(video_dec_ctx!=NULL)
    	avcodec_close(video_dec_ctx);
    if(ifmt_ctx != NULL)
    	avformat_close_input(&ifmt_ctx);
    if(decode_frame!=NULL)
    {
        av_frame_free(&decode_frame);
        decode_frame = NULL;
    }
	if(scale_frame != NULL)
	{
		av_frame_free(&scale_frame);
		scale_frame = NULL;
	}
    return 0;

}
