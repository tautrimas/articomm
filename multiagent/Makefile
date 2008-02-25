CXXFLAGS = -Wall -g -fmessage-length=0 -march=athlon-xp

OBJS =		main.o

LIBS =    -lpthread

TARGET =	multiagent

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
