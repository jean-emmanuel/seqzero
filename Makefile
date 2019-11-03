CXX = g++
CXXFLAGS = -g -O0 $(shell pkg-config --cflags json-c liblo jack) -Wall #-Wextra
LDFLAGS = $(shell pkg-config --libs json-c liblo jack)
SOURCES = $(wildcard src/*.cpp)
OBJ = $(SOURCES:.cpp=.o)
PROG = seqzero

all: $(PROG)

$(PROG): $(OBJ)
	$(info Edition des liens)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(info Compilation de $^ vers $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	-rm $(OBJ)
	-rm $(PROG)
