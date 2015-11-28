
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

INCLUDES = -I./include
CFLAGS = -fPIC -Wall -Wextra -O2 -g $(INCLUDES) $(DEFINES)
LDFLAGS = -shared -lUMP -lpthread

INSTALL_DIR = /usr/lib/
TARGET_LIB = librklayers.so
SRCS += rk_layers.c rk_video.c rk_memfunc.c
OBJS = $(SRCS:.c=.o)

CC = gcc
RM = rm -f
CP = cp -f

.PHONY: all
all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d) ${INSTALL_DIR}${TARGET_LIB}

.PHONY: install
install:
	${CP} ${TARGET_LIB} ${INSTALL_DIR}
