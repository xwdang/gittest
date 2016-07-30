#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#ifdef __cplusplus
}
#endif

  
//链接h264流
int joinmp4(char (*mp4files)[400] ,char * mp4,int length,int usefilter)
{
	//AVOutputFormat *ofmt = NULL;
	AVPacket pkt;
	AVStream *out_vstream = NULL;
	AVStream *out_astream = NULL;
	AVFormatContext *ofmt_ctx = NULL;
	int join_index = 0;
	AVBitStreamFilterContext* aacbsfc = NULL;
	long  last_video_pts = 0;
	long last_audio_pts = 0;
	long end_video_pts = 0;
	long end_audio_pts = 0;
	int videoindex_out = -1;
	int audioindex_out = -1;
    //Input AVFormatContext and Output AVFormatContext
    AVFormatContext * ifmt_ctx = NULL;

    int ret, i,retu =0,filter_ret=0;
    //	int fps;
    int videoindex_v=-1;
    int audioindex_a=-1;
    int frame_index=0;
    int64_t cur_pts_v=0,cur_pts_a=0;
    //set file path
    char *in_filename = mp4files[join_index];
    char *out_filename = mp4;
    int is_init_output = 0;
  
joinone:
    //Input AVFormatContext and Output AVFormatContext
    ifmt_ctx = NULL;

    ret = 0; i = 0;retu =0;filter_ret=0;
    //	int fps;
    videoindex_v=-1;
    audioindex_a=-1;
    frame_index=0;
    cur_pts_v=0;cur_pts_a=0;
    //set file path
    in_filename = mp4files[join_index];
    out_filename = mp4;


	//register before use
	av_register_all();
	//open Input and set avformatcontext
	if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		retu = -1;//-1 mean audio file opened failed
		
		goto end;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {

		retu = -3; //-3 mean get video info failed
		goto end;
	}

	//open Output
	if(join_index == 0)
	{
		avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
		if (!ofmt_ctx) {
			retu = -5;
			goto end;
		}
	}

	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		//Create output AVStream according to input AVStream
		if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			AVStream *in_stream = ifmt_ctx->streams[i];
			videoindex_v=i;

			if(join_index == 0)
			{
				out_vstream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
				videoindex_out=out_vstream->index;
				//Copy the settings of AVCodecContext
				if (avcodec_copy_context(out_vstream->codec, in_stream->codec) < 0) {
					retu = -7;
					goto end;
				}
				out_vstream->codec->codec_tag = 0;
				if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
					out_vstream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			}
			else
			{
				out_vstream->duration += in_stream->duration;
				//printf("duration = %ld\n",out_vstream->duration);
			}
			if (!out_vstream) {
				retu = -6;
				goto end;
			}
			
		}
		else if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO)
		{
			AVStream *in_stream = ifmt_ctx->streams[i];
			audioindex_a=i;

			if(join_index == 0)
			{
				out_astream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
				audioindex_out=out_astream->index;
				//Copy the settings of AVCodecContext
				if (avcodec_copy_context(out_astream->codec, in_stream->codec) < 0) {
					retu = -7;
					goto end;
				}
				out_astream->codec->codec_tag = 0;
				if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
					out_astream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			}
			else
			{
				out_astream->duration += in_stream->duration;
				//printf("duration = %ld\n",out_astream->duration);
			}
			if (!out_astream) {
				retu = -6;
				goto end;
			}	
		}
	}

	if(join_index == 0)
	{
			//Open output file
		if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
			if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
				retu = -10;
				goto end;
			}
		}
		//Write file header
		if (avformat_write_header(ofmt_ctx, NULL) < 0) {
			retu = -11;
			goto end;
		}
	}
	if(usefilter&& aacbsfc == NULL)
		aacbsfc = av_bitstream_filter_init("aac_adtstoasc");

	int have_write = 0;
	while(av_read_frame(ifmt_ctx, &pkt) >= 0)
	{
		if(pkt.stream_index==videoindex_v){

			pkt.pts+=end_video_pts;
			last_video_pts = pkt.pts;
		}
		else
		{
			pkt.pts+=end_audio_pts;
			last_audio_pts = pkt.pts;
		}
		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			av_free_packet(&pkt);
			break;
		}
		else
		{
			have_write = 1;
		}
		//av_packet_unref(&pkt);
			//av_interleaved_write_frame(ofmt_ctx, &pkt);
		av_free_packet(&pkt);
	}
	end_audio_pts = last_audio_pts;
	end_video_pts = last_video_pts;
	if(have_write)
		join_index++;

end:
	
	avformat_close_input(&ifmt_ctx);

    avformat_free_context(ifmt_ctx);

	if (retu < 0) {
		printf("error: ret = %d\n",retu);
		join_index = length;
	}
	if(join_index < length)
		goto joinone;
	
	if(usefilter&&aacbsfc!=NULL)
		av_bitstream_filter_close(aacbsfc);
	/* close output */
	if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
	{
		av_write_trailer(ofmt_ctx);
		avio_close(ofmt_ctx->pb);
		avformat_free_context(ofmt_ctx);
	}	
	return retu;
}
//拼接h264