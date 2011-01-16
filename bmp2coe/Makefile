# install libsdl1.2-dev libsdl-mixer1.2-dev

.SUFFIXES:
.SECONDARY:
.SECONDEXPANSION:

SHELL = /bin/sh
RM = rm -f

CC = gcc
CFLAGS = -g -fmessage-length=0
CPPFLAGS := -Wall -Wextra -MMD -MP -Wno-unused $(strip $(shell pkg-config sdl --cflags))

LD = gcc
LDFLAGS = -rdynamic
LDLIBS := $(strip $(shell pkg-config sdl --libs))

TARGETS = bmp2coe.x

bmp2coe.x_OBJS = bmp2coe.o

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	-$(RM) *.x *.o *.d

%.x: $$(%.x_OBJS)
	$(LD) $^ -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.c Makefile
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

%.d: ;

-include $(foreach TARGET, $(TARGETS), $($(TARGET)_OBJS:%.o=%.d))
