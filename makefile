#compiler define
CC := gcc

# compiler flags:
  # -g     	adds debugging information to the executable file
  # -Wall  	turns on most, but not all, compiler warnings
CFLAGS := -g -Wall

# soruce file defines
SOURCES := client.c server.c
PROGRAMS := $(patsubst %.c, %, $(SOURCES))
SUFFIX := .exe

#binary and object defines
BINS := $(patsubst %, %$(SUFFIX), $(PROGRAMS))
OBJS := $(patsubst %, %.o, $(PROGRAMS))

all : $(BINS)

.SECONDEXPANSION:
OBJ = $(patsubst %$(SUFFIX), %.o, $@)
BIN = $(patsubst %$(SUFFIX), %, $@)

#actual compiling 
%$(SUFFIX) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

## cleanups
clean:
	rm $(PROGRAMS) 
	rm -rf *~

rebuild: clean all
