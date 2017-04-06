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

test_mem_df: test_mem_df.o dflayer.o

test_mem_df.o: mem.h test_mem.c
	$(CC) $(CFLAGS) -DDEFRAG_LAYER=1 -o test_mem_df.o -c test_mem.c
