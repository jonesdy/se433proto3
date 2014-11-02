CC=gcc
CFLAGS=-Wall -c -fpic
LIBFLAGS=-shared -o
LIBRARY=face_io_lib
EXECUTABLE=face_pss.out

all:lib exe

exe:face_pss.o repl.o
	LD_PATH=. $(CC) -o $(EXECUTABLE) $^ -Wl,-rpath,. lib$(LIBRARY).so

lib:face_io_api.o config_parser.o
	$(CC) $(LIBFLAGS) lib$(LIBRARY).so $^

%.o:%.c
	$(CC) $(CFLAGS) $<
repl.o:repl.c
	$(CC) $(CFLAGS) repl.c -DCOLOR

clean:
	rm -rf *o $(EXECUTABLE) $(LIBRARY) *.o
