DEBUG ?=y

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
DEFINES += -DDEBUG
endif

INCLUDES = -I./include
CFLAGS = -fPIC -Wall -Wextra -O2 -g $(INCLUDES) $(DEFINES)
LDFLAGS = -shared -lUMP -lpthread

INSTALL_DIR = /usr/lib/
TARGET_LIB = librklayers.so
SRCS += rk_layers.c rk_video.c rk_memfunc.c chroma_neon.S

DEFINES += -DLIBNAME=$(TARGET_LIB)

OBJS = $(addsuffix .o,$(basename $(SRCS)))
DEP = $(addsuffix .d,$(basename $(SRC)))

CC = gcc
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
