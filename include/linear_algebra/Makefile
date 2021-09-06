CXX         = g++


#Default Flags (we prefer -std=c++17 but Mac/Xcode/Clang doesn't support)
CXXFLAGS = -std=c++1z -Wconversion -Wall -Werror -Wextra -pedantic 
run: test
	./Mtest
test:
	$(CXX) $(CXXFLAGS) -DDEBUG Matrix_test.cpp -o Mtest
clean: 
	rm Mtest
.PHONY: test run clean
