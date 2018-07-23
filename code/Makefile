EXECUTABLE = simpira
SOURCES    = $(wildcard *.cpp)
OBJECTS    = $(patsubst %,%,${SOURCES:.cpp=.o})
CXX        = g++
CXXFLAGS   = -Wall -O0 -g -c -msse2 -msse -march=native -maes -std=c++11 -o
LDFLAGS    = -pthread
LDLIBS     =
#-------------------------------------------------------------------------------


#make executable
all: $(EXECUTABLE)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $@ $< -MMD -MF ./$@.d

#link Objects
$(EXECUTABLE) : $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

run:
	./simpira $T

#make clean
clean:
	rm -f ./*.o
	rm -f ./*.o.d
	rm -f ./*.bin
	rm -f $(EXECUTABLE)

#The dependencies:
-include $(wildcard *.d)
