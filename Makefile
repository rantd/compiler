CC=gcc
CFLAGS=-c
SOURCES=cogen_main.c cogen.c parser.c tokenizer.c syntree.c util.c list.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=cogen

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean :
	rm -f *.o cogen
