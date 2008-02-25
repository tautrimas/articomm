CXXFLAGS = -Wall -g -fmessage-length=0 -march=athlon-xp

OBJS =		main.o

LIBS =    -lpthread

TARGET =	multiagent

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
evolution:
	g++ main.cpp -o main -O3 -Wall -lpthread
animation:
	g++ animateagent.cpp -o animateagent -lX11 -lXi -lXmu -lglut -lGL -lGLU -lm -O2
