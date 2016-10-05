SHELL=/bin/sh
CXX=gcc
CXXLD=gcc
MOC=moc

CXXFLAGS := -I. -pipe -I/usr/include/x86_64-linux-gnu/qt5/QtOpenGL -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtPrintSupport -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtXml -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtSvg -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtGui -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtNetwork -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtCore -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/local/lib/qwt/include -I/usr/X11R6/include -I/usr/include/qwt -I/usr/local/qwt-6.1.3 -I/usr/local/qwt-6.1.3/include -w -O3 -std=c++11 -fPIC
LDFLAGS := -lQt5OpenGL -lQt5PrintSupport -lQt5Xml -lQt5Svg -lQt5Widgets -lQt5Gui -lQt5Network -lQt5Core -lqwt -lgsl -lgslcblas -lm -ldl -lpthread -lGL -shared

TARGET=librtplot.so
HEADERS=$(shell find . -type f -name "*.h")
SOURCES=$(shell find . -type f -name "*.cpp")
OBJECTS=$(SOURCES:.cpp=.o)
MOC_SOURCES=$(HEADERS:.h=.moc.cpp)
MOC_OBJECTS=$(MOC_SOURCES:.moc.cpp=.moc.o)

all: $(TARGET)

$(TARGET): $(OBJECTS) $(MOC_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ -lc
#	$(CXX) -shared -fPIC -Wl,-soname,$(TARGET).2 -o $(TARGET).2.1 $(OBJECTS) -lc

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.moc.cpp: %.h
	$(MOC) $< -o $@


clean: 
	rm -f $(TARGET)
	rm -f $(OBJECTS)
	rm -f $(OBJECTS:.o=.lo)
	rm -f $(OBJECTS:.o=.la)
