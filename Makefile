DEBUG ?=n

ENABLE_RGA ?=n
ENABLE_IPP ?=n

ifeq ($(ENABLE_IPP),y)
DEFINES += -DIPP_ENABLE
SRCS += ipp_func.c
endif

ifeq ($(ENABLE_RGA),y)
DEFINES += -DRGA_ENABLE
SRCS += rga_func.c
endif

ifeq ($(DEBUG),y)
DEFINES += -g -O0 -DDEBUG
else
DEFINES += -O3
endif

INCLUDES = -I./include -I/media/filez1/src/OpenELEC.tv-master/build.Stalker-MK903V_fb.arm-1.0-devel/toolchain/armv7a-Stalker-linux-gnueabi/sysroot/usr/include/libdrm
CFLAGS = -fPIC -Wall -Wextra $(INCLUDES) $(DEFINES)
LDFLAGS = -shared -lpthread -ldrm

INSTALL_DIR = /usr/lib/
TARGET_LIB = librklayers.so
SRCS += rk_layers.c rk_video.c rk_memfunc.c chroma_neon.S

DEFINES += -DLIBNAME=$(TARGET_LIB)

OBJS = $(addsuffix .o,$(basename $(SRCS)))
DEP = $(addsuffix .d,$(basename $(SRC)))

CC = armv7a-Stalker-linux-gnueabi-gcc
RM = rm -f
CP = cp -f

.PHONY: all
all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(DEP):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

%.o: %.S                                                                               
	$(CC) -c $< -o $@ 

include $(wildcard $(DEP))

.PHONY: clean
clean:
	${RM} ${TARGET_LIB} ${OBJS} $(DEP) ${INSTALL_DIR}${TARGET_LIB}

.PHONY: install
install: ${TARGET_LIB}
	${CP} ${TARGET_LIB} ${INSTALL_DIR}
