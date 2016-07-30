#include <stdio.h>

extern "C"
{

#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

  
//链接h264流
int joinmp4(char (*h264file)[400] ,char (*aacfile)[400],char * mp4,int length,int usefilter)
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
    AVFormatContext * ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL;

    int ret, i,retu =0,filter_ret=0;
    //	int fps;
    int videoindex_v=-1;
    int audioindex_a=-1;
    int frame_index=0;
    int64_t cur_pts_v=0,cur_pts_a=0;
    //set file path
    char *in_filename_v = h264file[join_index];
    char *in_filename_a = aacfile[join_index];
    char *out_filename = mp4;
joinone:
    //Input AVFormatContext and Output AVFormatContext
    ifmt_ctx_v = NULL;
    ifmt_ctx_a = NULL;

    ret = 0; i = 0;retu =0;filter_ret=0;
    //	int fps;
    videoindex_v=-1;
    audioindex_a=-1;
    frame_index=0;
    cur_pts_v=0;cur_pts_a=0;
    //set file path
    in_filename_v = h264file[join_index];
    in_filename_a = aacfile[join_index];
    out_filename = mp4;

	//register before use
	av_register_all();
	//open Input and set avformatcontext
	if ((ret = avformat_open_input(&ifmt_ctx_a, in_filename_a, 0, 0)) < 0) {
		retu = -1;//-1 mean audio file opened failed
		
		goto end;
	}
	if ((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v, 0, 0)) < 0) {
		retu = -2; //-2 mean video file opened failed
		
		goto end;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {

		retu = -3; //-3 mean get video info failed
		goto end;
	}


	if ((ret = avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) {
		retu = -4;//-4 mean get audio info failed
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

	//ofmt = ofmt_ctx->oformat;
	//find all video stream input type
	for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
		//Create output AVStream according to input AVStream
		if(ifmt_ctx_v->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			AVStream *in_stream = ifmt_ctx_v->streams[i];
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
			break;
		}
	}

	//find all audio stream input type
	for (i = 0; i < ifmt_ctx_a->nb_streams; i++) {
		//Create output AVStream according to input AVStream
		if(ifmt_ctx_a->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
			AVStream *in_stream = ifmt_ctx_a->streams[i];
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
			break;
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


	while (true) {
		AVFormatContext *ifmt_ctx;
		int stream_index=0;
		AVStream *in_stream, *out_stream;
		//Get an AVPacket
		if(av_compare_ts(cur_pts_v,ifmt_ctx_v->streams[videoindex_v]->time_base,cur_pts_a,
					ifmt_ctx_a->streams[audioindex_a]->time_base) <= 0)
		{
			ifmt_ctx=ifmt_ctx_v;
			stream_index=videoindex_out;
			if(av_read_frame(ifmt_ctx, &pkt) >= 0){

				do{
					in_stream  = ifmt_ctx->streams[pkt.stream_index];
					out_stream = out_vstream;
					if(pkt.stream_index==videoindex_v){

						//Simple Write PTS
						if(pkt.pts==AV_NOPTS_VALUE){

							//Write PTS
							AVRational time_base1=in_stream->time_base;
							//Duration between 2 frames (us)
							int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
							//Parameters
							pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts=pkt.pts;
							pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							frame_index++;
						}
						cur_pts_v=pkt.pts;
						break;
					}
				}
				while(av_read_frame(ifmt_ctx, &pkt) >= 0);
			}
			else
			{
				//printf("pkt.duration = %ld\n",pkt.duration);
				join_index++;
				end_video_pts = last_video_pts;
				end_audio_pts = last_audio_pts;

					break;
			}
		}
		else
		{
			ifmt_ctx=ifmt_ctx_a;
			stream_index=audioindex_out;
			if(av_read_frame(ifmt_ctx, &pkt) >= 0){
				do
				{
					in_stream  = ifmt_ctx->streams[pkt.stream_index];
					out_stream = out_astream;
					if(pkt.stream_index==audioindex_a)
					{
						//Simple Write PTS
						if(pkt.pts==AV_NOPTS_VALUE)
						{
							//Write PTS
							AVRational time_base1=in_stream->time_base;
							//Duration between 2 frames (us)
							int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
							//Parameters
							pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts=pkt.pts;
							pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							frame_index++;
						}
						cur_pts_a=pkt.pts;
						break;
					}
				}
				while(av_read_frame(ifmt_ctx, &pkt) >= 0);
			}
			else
			{
				join_index++;
				end_video_pts = last_video_pts;
				end_audio_pts = last_audio_pts;

				break;
			}

		}
		if(usefilter)
			filter_ret = av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data,&pkt.size, pkt.data, pkt.size, 0);
		if(filter_ret)
		{
			retu = -10;
			goto end;

		}
		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,(enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,(enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);


		pkt.pos = -1;
		pkt.stream_index=stream_index;
		if(pkt.stream_index == audioindex_out)
		{
			pkt.pts += end_audio_pts;
			pkt.dts += end_audio_pts;
			last_audio_pts = pkt.pts+pkt.duration;
		//	printf("audio pts = %lld ,audio dts = %lld\n",pkt.pts,pkt.dts);
		}
		else
		{
			pkt.pts += end_video_pts;
			pkt.dts += end_video_pts;
			last_video_pts = pkt.pts+pkt.duration;
		}


		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			av_free_packet(&pkt);
			break;
		}
		//av_packet_unref(&pkt);
			//av_interleaved_write_frame(ofmt_ctx, &pkt);
		av_free_packet(&pkt);
	}


end:


	avformat_close_input(&ifmt_ctx_v);
	avformat_close_input(&ifmt_ctx_a);


    avformat_free_context(ifmt_ctx_v);
    avformat_free_context(ifmt_ctx_a);
	if (ret < 0 && ret != AVERROR_EOF) {
	}
	if(join_index < length)
		goto joinone;
	
	av_write_trailer(ofmt_ctx);

	
	if(usefilter)
		av_bitstream_filter_close(aacbsfc);
	/* close output */
	if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	return retu;
}
//拼接h264