.phony all:
all: ACS

ACS: ACS.c
	gcc ACS.c -g -std=c99 -D_GNU_SOURCE=1 -pthread -lreadline -lhistory -o ACS

.PHONY clean:
clean:
	-rm -rf *.o *.exe
