CC = g++
CXXFLAGS = $(shell pkg-config --cflags json-c liblo jack) -Wall -Wextra
LDFLAGS = $(shell pkg-config --libs json-c liblo jack)
SOURCES = $(wildcard src/*.cpp)
OBJ = $(SOURCES:.cpp=.o)
PROG = seqzero2

all: $(PROG)

$(PROG): $(OBJ)
	$(info Edition des liens)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(info Compilation de $^ vers $@)
	$(CC) $(CXXFLAGS) -c -o $@ $^

clean:
	-rm $(OBJ)
	-rm $(PROG)
