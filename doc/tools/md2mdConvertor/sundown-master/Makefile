# Makefile

# Copyright (c) 2009, Natacha Porté
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

DEPDIR=depends

# "Machine-dependant" options
#MFLAGS=-fPIC

#CFLAGS=-c -g -O0 -fPIC -Wall -Werror -Wsign-compare -Isrc -Ihtml
INCLUDE_DIR=\
        /home/users/tanzhaobo/sequoiadb/SequoiaDB/engine/include

CFLAGS=-c -g -O0 -fPIC -Wall -Wsign-compare -Isrc -Ihtml -I$(INCLUDE_DIR)
LDFLAGS=-g -O3 -Wall -Werror 
CC=gcc


SUNDOWN_SRC=\
	src/markdown.o \
	src/stack.o \
	src/buffer.o \
	src/autolink.o \
	src/pandoc.o

all:		libpandoc_markdown.so pandoc_markdown

.PHONY:		all clean

# libraries

libpandoc_markdown.so:	libpandocmarkdown.so.1
	ln -f -s $^ $@

libpandocmarkdown.so.1: $(SUNDOWN_SRC)
	$(CC) $(LDFLAGS) -shared -Wl $^ -o $@

# executables

pandoc_markdown:	src/pandoc_markdown.o $(SUNDOWN_SRC)
	$(CC) $(LDFLAGS) $^ -o $@

## perfect hashing
#html_blocks: src/html_blocks.h
#
#src/html_blocks.h: html_block_names.txt
#	gperf -N find_block_tag -H hash_block_tag -C -c -E --ignore-case $^ > $@


# housekeeping
clean:
	rm -f src/*.o html/*.o examples/*.o
	rm -f libpandoc_markdown.so libpandoc_markdown.so.1 pandoc_markdown
	rm -f pandoc_markdown.exe
	rm -rf $(DEPDIR)


# dependencies

include $(wildcard $(DEPDIR)/*.d)


# generic object compilations

%.o:	src/%.c examples/%.c html/%.c
	@mkdir -p $(DEPDIR)
	@$(CC) -MM $< > $(DEPDIR)/$*.d
	$(CC) $(CFLAGS) -o $@ $<

