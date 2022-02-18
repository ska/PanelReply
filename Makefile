#Makefile
#Inserire i file *.o come contenuto della var OBJECTS

OBJECTS = main.o netinfo.o

#nome eseguibile
PROG_NAME    = PanelReply

all: progs

progs:	$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(PROG_NAME) $(LIBS)

call: clean progs

clean:
	@-rm $(PROG_NAME) $(OBJECTS)



.PHONY : all clean call
