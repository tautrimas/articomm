CXXFLAGS = -Wall -fmessage-length=0

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
gprof:
	g++ main.cpp -o profilation -g -pg -lpthread -O2 && ./profilation && gprof profilation -b > profile.txt
