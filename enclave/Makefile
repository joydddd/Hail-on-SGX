## EECS 281 Advanced Makefile

# How to use this Makefile...
###################
###################
##               ##
##  $ make help  ##
##               ##
###################
###################

# IMPORTANT NOTES:
#   1. Set EXECUTABLE to the command name from the project specification.
#   2. To enable automatic creation of unit test rules, your program logic
#      (where main() is) should be in a file named project*.cpp or
#      specified in the PROJECTFILE variable.
#   3. Files you want to include in your final submission cannot match the
#      test*.cpp pattern.

#######################
# TODO (begin) #
#######################

BOOST_ROOT = /usr/local/lib/boost_1_76_0/


#######################
# TODO (end) #
#######################

EXECUTABLE  = gwas

PROJECTFILE = $(or $(wildcard project*.cpp $(EXECUTABLE).cpp), main.cpp)

# designate which compiler to use
CXX         = g++

# list of test drivers (with main()) for development
TESTSOURCES = $(wildcard test*.cpp)
TESTOBJS = $(TESTSOURCES:%.cpp=%.o)
# names of test executables
TESTS       = $(TESTSOURCES:%.cpp=%)
TESTOUT     = $(TESTSOURCES:%.cpp=%.out)

# list of sources used in project
SOURCES     = $(wildcard *.cpp)
SOURCES     := $(filter-out $(TESTSOURCES), $(SOURCES))
# list of objects used in project
OBJECTS     = $(SOURCES:%.cpp=%.o)

#Default Flags (we prefer -std=c++17 but Mac/Xcode/Clang doesn't support)
# CXXFLAGS = -std=c++1z -Wconversion -Wall -Werror -Wextra -pedantic 
CXXFLAGS = -std=c++1z -pedantic 
CXXFLAGS += -I $(BOOST_ROOT)

# make release - will compile "all" with $(CXXFLAGS) and the -O3 flag
#                also defines NDEBUG so that asserts will not check
release: CXXFLAGS += -O3 -DNDEBUG
release: $(EXECUTABLE)


# make debug - will compile sources with $(CXXFLAGS) and the -g3 flag
#              also defines DEBUG, so "#ifdef DEBUG /*...*/ #endif" works
debug: CXXFLAGS += -g3 -DDEBUG
debug:
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(EXECUTABLE)_debug


# Build all executables
all: release

$(EXECUTABLE): $(OBJECTS)
ifeq ($(EXECUTABLE), executable)
	@echo Edit EXECUTABLE variable in Makefile.
	@echo Using default a.out.
	$(CXX) $(CXXFLAGS) $(OBJECTS)
else
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXECUTABLE)
endif

# Automatically generate any build rules for test*.cpp files
define make_tests
    HDRS = $$(wildcard *.h *.hpp)
    $(1): CXXFLAGS += -g3 -DDEBUG
    $(1): $$(OBJECTS) $(1).cpp
	$$(CXX) $$(CXXFLAGS) $$(OBJECTS) $(1).o -o $(1)
	./$(1)
endef
$(foreach test, $(TESTS), $(eval $(call make_tests,$(test))))

$(TESTS):$(TESTOBJS) $(OBJECTS)

alltests:$(TESTS)

# rule for creating objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

# make clean - remove .o files, executables, tarball
clean:
	rm -f $(OBJECTS) $(EXECUTABLE) $(EXECUTABLE)_debug $(EXECUTABLE)_profile \
      $(TESTS)  *.o
	rm -Rf *.dSYM *.tsv *.out

#
# % g++ -MM *.cpp
#
# ADD YOUR OWN DEPENDENCIES HERE

buffer.o: buffer.cpp gwas.h type.h ../gwas.h ../gwas_error.h \
 ../linear_algebra/Matrix.h enclave.h
enclave.o: enclave.cpp enclave.h type.h ../gwas.h ../gwas_error.h gwas.h \
 ../linear_algebra/Matrix.h
linear_regression.o: linear_regression.cpp gwas.h type.h ../gwas.h \
 ../gwas_error.h ../linear_algebra/Matrix.h
logistic_regression.o: logistic_regression.cpp gwas.h type.h ../gwas.h \
 ../gwas_error.h ../linear_algebra/Matrix.h
test_enclave.o: test_enclave.cpp gwas.h type.h ../gwas.h ../gwas_error.h \
 ../linear_algebra/Matrix.h enclave.h
test_linear_reg.o: test_linear_reg.cpp gwas.h type.h ../gwas.h \
 ../gwas_error.h ../linear_algebra/Matrix.h
test_logistic_regression.o: test_logistic_regression.cpp gwas.h type.h \
 ../gwas.h ../gwas_error.h ../linear_algebra/Matrix.h
######################
# TODO (end) #
######################

# these targets do not create any files
.PHONY: all release debug profile static clean alltests partialsubmit \
        fullsubmit ungraded sync2caen help identifier
# disable built-in rules
.SUFFIXES:


