# This file is dual-licensed.  Choose whichever licence you want from
# the two licences listed below.
#
# The first licence is a regular 2-clause BSD licence.  The second licence
# is the CC-0 from Creative Commons. It is intended to release Monocypher
# to the public domain.  The BSD licence serves as a fallback option.
#
# SPDX-License-Identifier: BSD-2-Clause OR CC0-1.0
#
# ------------------------------------------------------------------------
#
# Copyright (c) 2017-2019, Loup Vaillant
# Copyright (c) 2017, 2019, Fabio Scotoni
# All rights reserved.
#
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ------------------------------------------------------------------------
#
# Written in 2017-2019 by Loup Vaillant and Fabio Scotoni
#
# To the extent possible under law, the author(s) have dedicated all copyright
# and related neighboring rights to this software to the public domain
# worldwide.  This software is distributed without any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication along
# with this software.  If not, see
# <https://creativecommons.org/publicdomain/zero/1.0/>
.POSIX:
.SUFFIXES:

CC           ?= gcc -std=c99
CFLAGS       ?= -pedantic -Wall -Wextra -O3
DESTDIR      ?=
PREFIX       ?= /usr/local
LIBDIR       ?= $(PREFIX)/lib
INCLUDEDIR   ?= $(PREFIX)/include
PKGCONFIGDIR ?= $(LIBDIR)/pkgconfig
MANDIR       ?= $(PREFIX)/share/man/man3
SONAME        = libmonocypher.so.4
PORTABLE     ?= 1
HARDEN       ?= 0
SANITIZE     ?=
SIZE         ?= 0
STRERROR     ?= 0
RNG_DIAGNOSTICS ?= 0
ARGON2_THREADS ?= 0
BLAKE3_THREADS ?= 0
CHACHA20_THREADS ?= 0
FUZZ_CC ?= clang
FUZZ_CFLAGS ?= -std=c99 -g -O1 -fsanitize=fuzzer,address,undefined
FUZZ_LDFLAGS ?=
FUZZ_BIN_DIR ?= tests/fuzz/bin
FUZZ_CORPUS_DIR ?= tests/fuzz/corpus
FUZZ_RUNS ?= 1000
FUZZ_I = -I src -I src/optional
FUZZERS = fuzz_aead fuzz_aead_safe fuzz_x25519 fuzz_eddsa fuzz_argon2 \
          fuzz_hashes fuzz_poly1305 fuzz_elligator
FUZZ_TARGETS = $(addprefix $(FUZZ_BIN_DIR)/,$(FUZZERS))

ifeq ($(PORTABLE),0)
CFLAGS       += -march=native
endif

ifeq ($(HARDEN),1)
CFLAGS       += -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fno-omit-frame-pointer
LDFLAGS      += -Wl,-z,relro -Wl,-z,now
endif

ifneq ($(SANITIZE),)
CFLAGS       += -fsanitize=$(SANITIZE)
LDFLAGS      += -fsanitize=$(SANITIZE)
endif

ifeq ($(SIZE),1)
CFLAGS       += -Os -DBLAKE2_NO_UNROLLING
endif

ifeq ($(STRERROR),1)
CFLAGS       += -DMONOCYPHER_STRERROR=1
endif

ifeq ($(RNG_DIAGNOSTICS),1)
CFLAGS       += -DMONOCYPHER_RNG_DIAGNOSTICS=1
endif

ifeq ($(ARGON2_THREADS),1)
CFLAGS       += -DMONOCYPHER_ARGON2_PTHREADS=1 -pthread
LDFLAGS      += -pthread
endif

ifeq ($(BLAKE3_THREADS),1)
CFLAGS       += -DMONOCYPHER_BLAKE3_PTHREADS=1 -pthread
LDFLAGS      += -pthread
endif

ifeq ($(CHACHA20_THREADS),1)
CFLAGS       += -DMONOCYPHER_CHACHA20_PTHREADS=1 -pthread
LDFLAGS      += -pthread
endif

.PHONY: all library static-library dynamic-library  \
        install install-lib install-pc install-doc  \
        check test tis-ci ctgrind harden sanitize size \
        fuzz fuzz-run fuzz-corpus \
        clean uninstall dist

##################
## Main targets ##
##################
all  : library doc/man3/intro.3monocypher
check: test

test: test.out
	./test.out

tis-ci: tis-ci.out
	./tis-ci.out

ctgrind: ctgrind.out
	valgrind ./ctgrind.out

harden:
	$(MAKE) HARDEN=1

sanitize:
	$(MAKE) SANITIZE=address,undefined

size:
	$(MAKE) SIZE=1

fuzz: $(FUZZ_TARGETS)

fuzz-corpus:
	tests/fuzz/gen_corpus.sh $(FUZZ_CORPUS_DIR)

fuzz-run: fuzz fuzz-corpus
	@for f in $(FUZZ_TARGETS); do \
		corpus="$(FUZZ_CORPUS_DIR)/$$(basename $$f)"; \
		if [ -d "$$corpus" ]; then \
			$$f -runs=$(FUZZ_RUNS) "$$corpus"; \
		else \
			$$f -runs=$(FUZZ_RUNS); \
		fi; \
	done

clean:
	rm -rf lib/ doc/html/ doc/man3/
	rm -f *.out
	rm -rf tests/fuzz/bin

#############
## Install ##
#############
install: install-lib install-pc install-doc

install-lib: library
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp -P lib/libmonocypher.a lib/libmonocypher.so* $(DESTDIR)$(LIBDIR)
	cp -P src/monocypher.h                          $(DESTDIR)$(INCLUDEDIR)
	cp -P src/optional/monocypher-ed25519.h         $(DESTDIR)$(INCLUDEDIR)

install-pc: monocypher.pc
	mkdir -p $(DESTDIR)$(PKGCONFIGDIR)
	sed "s|PREFIX|$(PREFIX)|"  monocypher.pc \
	    > $(DESTDIR)$(PKGCONFIGDIR)/monocypher.pc

install-doc: doc/man3/intro.3monocypher
	mkdir -p $(DESTDIR)$(MANDIR)
	cp -PR doc/man3/*.3monocypher $(DESTDIR)$(MANDIR)

uninstall:
	rm -f $(DESTDIR)$(LIBDIR)/libmonocypher.a
	rm -f $(DESTDIR)$(LIBDIR)/libmonocypher.so*
	rm -f $(DESTDIR)$(INCLUDEDIR)/monocypher.h
	rm -f $(DESTDIR)$(INCLUDEDIR)/monocypher-ed25519.h
	rm -f $(DESTDIR)$(PKGCONFIGDIR)/monocypher.pc
	rm -f $(DESTDIR)$(MANDIR)/*.3monocypher

##################
## Main library ##
##################
library: static-library dynamic-library
static-library : lib/libmonocypher.a
dynamic-library: lib/libmonocypher.so lib/$(SONAME)

MAIN_O=lib/monocypher.o lib/monocypher-ed25519.o
MAIN_I=-I src -I src/optional

lib/libmonocypher.a: $(MAIN_O)
	$(AR) cr $@ $(MAIN_O)

lib/libmonocypher.so: lib/$(SONAME)
	ln -sf `basename lib/$(SONAME)` $@

lib/$(SONAME): $(MAIN_O)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -Wl,-soname,$(SONAME) -o $@ $(MAIN_O)

lib/monocypher.o: src/monocypher.c src/monocypher.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(MAIN_I) -fPIC -c -o $@ src/monocypher.c

lib/monocypher-ed25519.o: src/optional/monocypher-ed25519.c \
                          src/optional/monocypher-ed25519.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(MAIN_I) -fPIC -c -o $@ src/optional/monocypher-ed25519.c

####################
## Test libraries ##
####################
TEST_COMMON=tests/utils.h src/monocypher.h src/optional/monocypher-ed25519.h
TEST_I=$(MAIN_I) -I tests

lib/utils.o: tests/utils.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_I) -fPIC -c -o $@ tests/utils.c

lib/test.o: tests/test.c $(TEST_COMMON) tests/vectors.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_I) -fPIC -c -o $@ tests/test.c

lib/tis-ci.o: tests/tis-ci.c $(TEST_COMMON) tests/tis-ci-vectors.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_I) -fPIC -c -o $@ tests/tis-ci.c

lib/ctgrind.o: tests/ctgrind.c $(TEST_COMMON)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_I) -fPIC -c -o $@ tests/ctgrind.c

######################
## Fuzzing targets  ##
######################
$(FUZZ_BIN_DIR)/fuzz_%: tests/fuzz/fuzz_%.c src/monocypher.c src/monocypher.h
	@mkdir -p $(FUZZ_BIN_DIR)
	$(FUZZ_CC) $(FUZZ_CFLAGS) $(FUZZ_I) $< src/monocypher.c \
		-o $@ $(FUZZ_LDFLAGS)

######################
## Test executables ##
######################
TEST_OBJ = lib/utils.o lib/monocypher.o lib/monocypher-ed25519.o

test.out: lib/test.o $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ lib/test.o $(TEST_OBJ)

tis-ci.out: lib/tis-ci.o $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ lib/tis-ci.o $(TEST_OBJ)

ctgrind.out: lib/ctgrind.o $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ lib/ctgrind.o $(TEST_OBJ)
# Remove lines below for the tarball

tests/vectors.h:
	@echo ""
	@echo "======================================================"
	@echo " I cannot perform the tests without the test vectors."
	@echo " You must generate them.  This requires Libsodium."
	@echo " The following will generate the test vectors:"
	@echo ""
	@echo "     $ cd tests/gen"
	@echo "     $ make"
	@echo ""
	@echo " Alternatively, you can grab an official release."
	@echo " It will include the test vectors, so you won't"
	@echo " need libsodium."
	@echo "======================================================"
	@echo ""
	exit 1

doc/man3/intro.3monocypher: \
	doc/crypto_aead_lock.3monocypher     doc/crypto_argon2.3monocypher       \
	doc/crypto_blake2b.3monocypher       doc/crypto_chacha20_djb.3monocypher \
	doc/crypto_ed25519_sign.3monocypher  doc/crypto_eddsa_sign.3monocypher   \
	doc/crypto_elligator_map.3monocypher doc/crypto_poly1305.3monocypher     \
	doc/crypto_sha512.3monocypher        doc/crypto_verify16.3monocypher     \
	doc/crypto_wipe.3monocypher          doc/crypto_x25519.3monocypher       \
	doc/intro.3monocypher
	doc/doc_gen.sh

dist: tests/vectors.h
	./dist.sh
