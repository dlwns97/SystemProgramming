CC=gcc
CFLAGS= -W -Wall

TARGET=20161647.out
OBJECTS=20161647.o

all : $(TARGET)

$(TARGET) : $(OBJECTS)
			  $(CC) $(CFLAGS) -o $@ $^
clean:
	  rm 20161647.out *.o
