CXX = g++
CXXFLAGS = -Wall -O2
TARGET = fks
SRC = src/fks.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

install:
	mkdir -p $(DESTDIR)/usr/bin
	mkdir -p $(DESTDIR)/usr/share/fks
	cp $(TARGET) $(DESTDIR)/usr/bin/
	cp src/ascii.txt $(DESTDIR)/usr/share/fks/

clean:
	rm -f $(TARGET)
