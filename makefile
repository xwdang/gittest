
# use pkg-config for getting CFLAGS and LDLIBS $(shell pkg-config --libs --static opencv) $(shell pkg-config --cflags opencv) $(shell pkg-config --cflags opencv)
$(shell export PKG_CONFIG_PATH=/home/chenglong/work/ffmpeg_lib/lib/pkgconfig/:$PKG_CONFIG_PA)
FFMPEG_LIBS=    libavdevice                        \
                libavformat                        \
                libavfilter                        \
                libavcodec                         \
                libswresample                      \
                libswscale                         \
                libavutil                          \


CFLAGS += -Wall -g   -Wno-write-strings -Wno-deprecated -Wno-sign-compare
CFLAGS += -I.

CFLAGS := $(shell pkg-config --cflags $(FFMPEG_LIBS))  $(CFLAGS)
LDLIBS := -L./lib/ -L/usr/local/lib/ $(shell pkg-config --libs $(FFMPEG_LIBS)) $(LDLIBS) -lm  \
-lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -lopencv_hal -lpng -lz \
 -lpng -lz -ldl -lm -lpthread -lrt

CXXFLAGS += $(CFLAGS)

EXAMPLES=      main

SRC_DATA=$(EXAMPLES) \
		joinmp4  \
		thumbnail

OBJS=$(addsuffix .o,$(SRC_DATA))
#OBJS=$(addsuffix .o,$(EXAMPLES) main)
# the following examples make explicit use of the math library
avcodec:           LDLIBS += -lm
decoding_encoding: LDLIBS += -lm
muxing:            LDLIBS += -lm
resampling_audio:  LDLIBS += -lm
#OBJS:$(addsuffix .c,$(EXAMPLES) main)
#	cc -I/usr/local/include   -Wall -g    filtering.c list.c -L/usr/local/lib -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil
SRC  := $(addsuffix .cpp,$(SRC_DATA))
#SRC  := $(addsuffix .c,$(EXAMPLES) main)
TARGET := main
.PHONY : clean all
all: $(TARGET)
$(TARGET):$(OBJS)
	g++ $(CFLAGS)  $(OBJS) $(LDLIBS)  -o $(TARGET)

clean-test:
	$(RM) test*.pgm test.h264 test.mp2 test.sw test.mpg
	@echo $(CFLAGS)
	@echo $(LDLIBS)
clean: clean-test
	$(RM) $(EXAMPLES) $(OBJS)
