SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
LDFLAGS = -lmingw32 -lavutil -lavformat -lavcodec -lswscale -lSDLmain -lSDL
CPPFLAGS = 
CXXFLAGS = $(CPPFLAGS) -W -Wall -g
CXX = g++
MAIN = vplayer

all: $(MAIN)

$(MAIN): $(OBJECTS)
	@echo Creating $@...
	@$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f *.o *.d $(MAIN)

