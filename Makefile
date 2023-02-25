CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=src/main.cpp src/mem.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=build/hvf_trainer

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
