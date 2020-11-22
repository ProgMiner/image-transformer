
CC = gcc
LD = gcc
CFLAGS = -std=c89 -pedantic-errors -Wall -Werror -g -O0 # -O2
LDFLAGS = -ldl -rdynamic

BUILDPATH = build
SOURCES = main.c ast.c value.c parser.c lexer.c interpreter.c image.c util.c stdlib.c bmp.c
HEADERS = ast.h value.h parser.h interpreter.h image.h util.h bmp.h
TARGET = image-transformer

OBJECTS = $(SOURCES:%.c=$(BUILDPATH)/%.o)

.PHONY: all build clean modules
.SUFFIXES:

all: build modules

clean:
	@+cd modules; make clean
	@rm -vrf $(BUILDPATH) 2> /dev/null; true
	@rm -v parser.h parser.c lexer.c $(TARGET) 2> /dev/null; true

build: $(TARGET)

modules:
	@+cd modules; make

%.lex:

%.y:

parser.h parser.c: parser.y
	bison --defines=parser.h -o parser.c parser.y

lexer.c: lexer.lex parser.h
	flex -o lexer.c lexer.lex

%.c:

$(OBJECTS): $(BUILDPATH)/%.o : %.c $(HEADERS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)
