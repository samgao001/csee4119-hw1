CC := gcc

# compiler flags:
  # -g     	adds debugging information to the executable file
  # -Wall  	turns on most, but not all, compiler warnings
CFLAGS := -g -Wall

# soruce file defines
SOURCES := client.c server.c
PROGRAMS := $(patsubst %.c, %, $(SOURCES))
SUFFIX := .exe

BINS := $(patsubst %, %$(SUFFIX), $(PROGRAMS))
OBJS := $(patsubst %, %.o, $(PROGRAMS))

all : $(BINS)

.SECONDEXPANSION:
OBJ = $(patsubst %$(SUFFIX), %.o, $@)
BIN = $(patsubst %$(SUFFIX), %, $@)

%$(SUFFIX) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

clean:
	$(RM) $(PROGRAMS)

rebuild: clean all
