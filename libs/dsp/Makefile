SHELL=/bin/sh
CXX=gcc
CXXLD=gcc
MOC=moc

CXXFLAGS := -I. -w -O3 -std=c++11 -fPIC
#LDFLAGS := -lgsl -lgslcblas -lm -ldl -lpthread -lGL -shared -module -avoid-version
LDFLAGS := -lgsl -lgslcblas -lm -ldl -lpthread -lGL -shared

TARGET=librtdsp.so
HEADERS=$(shell find . -type f -name "*.h")
SOURCES=$(shell find . -type f -name "*.cpp")
OBJECTS=$(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CXX) $(LDFLAGS) -o $@ $^ -lc
#	$(CXX) -shared -fPIC -Wl,-soname,$(TARGET).2 -o $(TARGET).2.1 $(OBJECTS) -lc

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean: 
	rm -f $(TARGET)
	rm -f $(OBJECTS)
	rm -f $(OBJECTS:.o=.lo)
	rm -f $(OBJECTS:.o=.la)
