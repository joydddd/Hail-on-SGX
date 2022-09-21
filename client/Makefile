# Based on https://hiltmon.com/blog/2013/07/03/a-simple-c-plus-plus-project-structure/
CC := g++
SRCDIR := src
BUILDDIR := build
SHAREDBUILDDIR := ../shared/build
TARGETDIR := bin
TESTDIR := tests
UNITTESTDIR := unit_tests
CONFIG:= client_config.json

SRCEXT := cpp
# List of source files for your thread library
SOURCES = $(shell find $(SRCDIR) -type f -name *.cpp)
# Generate the names of the thread library's object files
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
OBJECTS += $(shell find $(SHAREDBUILDDIR) -type f -name *.o)
CFLAGS := -g -Wall -std=c++14 -pthread
FSFLAGS := -ldl
INC := -I ../shared/include -I include
CRYPTFLAG := -lcryptopp
CURLFLAG := -lcurl

# list of test drivers (with main()) for development
TESTSOURCES := $(wildcard $(TESTDIR)/*test*.$(SRCEXT))
# names of test executables
TESTOBJECTS := $(patsubst $(TESTDIR)/%,%,$(TESTSOURCES:.$(SRCEXT)=))

ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
    detected_OS := Windows
else
    detected_OS := $(shell uname)
endif

# Use Mac library if on Mac
ifeq ($(detected_OS),Darwin)  # Important to not have a space after the comma
	LIBTHREAD = -lboost_thread-mt
	LIBCHRONO = -lboost_chrono-mt
else
	LIBTHREAD = -lboost_thread -lboost_system
	LIBCHRONO = -lboost_chrono
endif

all: shared client

# Generic rules for compiling a source file to an object file
${BUILDDIR}/%.o: ${SRCDIR}/%.${SRCEXT}
	@mkdir -p ${BUILDDIR}
	${CC} ${CFLAGS} ${INC} -c -o $@ $<

# Automatically generate any build rules for test*.cpp files
define make_tests
    $(1): $$(TESTDIR)/$(1).$$(SRCEXT)
	$$(CC) $$(CFLAGS) $$(INC) -o $$(TARGETDIR)/$(1).o $$^ $$(LIBTHREAD)
endef
$(foreach test,$(TESTOBJECTS),$(eval $(call make_tests,$(test))))

# Compile all test programs
tests: ${TESTOBJECTS}

shared:
	cd ../shared && $(MAKE) && cd ../client

# Compile the pager and tag this compilation
client: ${OBJECTS}
	@mkdir -p ${TARGETDIR}
	${CC} ${CFLAGS} ${INC} -o ${TARGETDIR}/$@ $^ ${LIBTHREAD} ${FSFLAGS} ${CRYPTFLAG} ${CURLFLAG}

run:
	./bin/client $(CONFIG)

clean:
	@echo "Cleaning..."
	rm -r ${BUILDDIR} ${TARGETDIR}
