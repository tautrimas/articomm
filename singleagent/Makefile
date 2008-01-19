# I am a comment, and I want to say that the variable CC will be
# the compiler to use.
CC=g++
# Hey!, I am comment number 2. I want to say that CFLAGS will be the
# options I'll pass to the compiler.
CFLAGS=-c -Wall
LDFLAGS=-O2

all: main
main: main.o
	$(CC) $(LDFLAGS) main.o -o main -O2
main.o: main.cpp animateagent.cpp environment.cpp header.cpp pool.cpp robot.cpp ann.cpp evolution.cpp main.cpp random.cpp simulation.cpp
	$(CC) $(CFLAGS) main.cpp
run:
	./main
animate: animateagent

animateagent: animateagent.o
	$(CC) animateagent.o -o animateagent -O2 -lX11 -lXi -lXmu -lglut -lGL -lGLU -lm
animateagent.o: animateagent.cpp
	$(CC) $(CFLAGS) animateagent.cpp
run2:
	./animateagent
clean:
	rm -f main animateagent *.o
clean2:
	rm -f *.o
