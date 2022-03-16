# Based on https://hiltmon.com/blog/2013/07/03/a-simple-c-plus-plus-project-structure/
CC := g++
SRCDIR := src
BUILDDIR := build
TARGETDIR := bin


SRCEXT := cpp
# List of source files for your thread library
SOURCES = $(shell find $(SRCDIR) -type f -name *.cpp)
# Generate the names of the thread library's object files
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g -Wall -std=c++14 -pthread
INC := -I ../shared -I include

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

all: clean shared

# Generic rules for compiling a source file to an object file
${BUILDDIR}/%.o: ${SRCDIR}/%.${SRCEXT}
	@mkdir -p ${BUILDDIR}
	${CC} ${CFLAGS} ${INC} -c -o $@ $<

# Compile the pager and tag this compilation
shared: ${OBJECTS}
	@mkdir -p ${BUILDDIR}
	$(CC) -c $(CFLAGS) $(OBJECTS)

clean:
	@echo "Cleaning..."
	@if [ -d ${BUILDDIR} ]; then rm -r ${BUILDDIR}; fi
