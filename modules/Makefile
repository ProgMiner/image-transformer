
CC = gcc
LD = gcc
CFLAGS = -std=c89 -pedantic-errors -Wall -Werror -fPIC -g -O0 # -O2
LDFLAGS = -shared -lm

BUILDPATH = build
SOURCES = rotate.c blur.c
HEADERS = ../image.h ../value.h

OBJECTS = $(SOURCES:%.c=$(BUILDPATH)/%.o)
TARGETS = $(OBJECTS:$(BUILDPATH)/%.o=%.so)

.PHONY: all build clean
.SUFFIXES:

all: build

clean:
	@-rm -vrf $(BUILDPATH) 2> /dev/null; true
	@-rm -v $(TARGETS) 2> /dev/null; true

build: $(TARGETS)

%.c:

$(OBJECTS): $(BUILDPATH)/%.o : %.c $(HEADERS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGETS): %.so: $(BUILDPATH)/%.o
	$(LD) -o $@ $^ $(LDFLAGS)
