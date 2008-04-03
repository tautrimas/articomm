CXXFLAGS = -Wall -fmessage-length=0 -g -O2

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
gdb:
	g++ main.cpp -o main -O3 -g -Wall -lpthread
animation:
	g++ animateagent.cpp -o animateagent -lX11 -lXi -lXmu -lglut -lGL -lGLU -lm -O2
gprof:
	g++ main.cpp -o profilation -g -pg -lpthread -O2 && ./profilation 1 && gprof profilation -b > profile.txt
