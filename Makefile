CC=gcc
CFLAGS=-Wall -c -fpic
EXECUTABLE=face_pss.out

all:config_parser.o face_io_api.o face_pss.o
	$(CC) -o $(EXECUTABLE) $^

%.o:%.c
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *o $(EXECUTABLE)
