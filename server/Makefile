# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

include config.mk
.PHONY: all build clean run simulate

OE_CRYPTO_LIB := mbedtls
export OE_CRYPTO_LIB

all: build

build:
	$(MAKE) -C enclave
	$(MAKE) -C host

debug:
	$(MAKE) -C enclave debug
	$(MAKE) -C host

tests:
	$(MAKE) -C enclave
	$(MAKE) -C host tests

runtests:
	$(MAKE) -C host runtests

clean:
	$(MAKE) -C enclave clean
	$(MAKE) -C host clean

run:
	$(MAKE) -C host run

