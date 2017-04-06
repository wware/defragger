PYDVER := $(shell python -c "import sys; print sys.version[:3]")
UNAME:=$(shell uname)
# PYBASE := $(shell which python | sed "s%/bin/python%%")
PYBASE := /usr
INCLUDES = -I$(PYBASE)/include/python$(PYDVER)
PYLIBDIR = $(PYBASE)/lib/python$(PYDVER)
CFLAGS += $(INCLUDES)
LDFLAGS = -L$(PYLIBDIR)/config -lm -lpython$(PYDVER)
LDSHARED = $(CC) -shared
CC=gcc
CFLAGS+=-Wall -g -O2 -fPIC
# CFLAGS+=-Wall -g -pg    # for collecting performance data

all: test_mem

clean:
	git clean -xdf

test_mem: test_mem.o mem.o

test_mem.o: mem.h
