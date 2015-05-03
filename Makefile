CC=g++
CFLAGS=-g -Wall -std=c++11

SRCS=\
main.cpp\
mml.cpp

OBJS=$(SRCS:.cpp=.o)
DEPS=$(SRCS:.cpp=.d)

INCS=-I.
LIBS=-lGL -lglut -lGLU

RM=rm -rf

grapher: $(OBJS)
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(INCS) $(LIBS)

%.o: %.cpp
	$(CC) $< -o $@ $(CFLAGS) -c $(INCS) -MP -MMD

clean:
	$(RM) grapher $(OBJS) $(DEPS)

-include $(DEPS)
