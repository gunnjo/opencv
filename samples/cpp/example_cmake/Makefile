CXX ?= g++

CXXFLAGS += -c -Wall $(shell pkg-config --cflags opencv)
LDFLAGS += $(shell pkg-config --libs --static opencv)
LFLAGS = $(LDFLAGS:-L/usr/local/lib=-L../../../Release/lib)

all: opencv_example

opencv_example: example.o; $(CXX) $< -o $@ $(LFLAGS)

%.o: %.cpp; $(CXX) $< -o $@ $(CXXFLAGS)

clean: ; rm -f example.o opencv_example
