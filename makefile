#Makefile
SERVER = server

all: $(SERVER)

$(SERVER): $(SERVER).c $(SERVER).h library.c library.h
	gcc -Wall -g -o $(SERVER) library.c library.h $(SERVER).c $(SERVER).h -lrt -lpthread
	
clean:
	rm $(SERVER)
