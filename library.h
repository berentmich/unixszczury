#define _GNU_SOURCE
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <ftw.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <aio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>
#include <pthread.h>

#define BACKLOG 3
#define MAX_PLAYERS 20
#define MAX_THREAD 20
#define NORMAL_MSG_SIZE 50
int sethandler(int sigNo, void (*handler)(int));
void sigchildhandler(int sigNo);
void error(const char* reason);
ssize_t saferead(int fd, char* buf, size_t count);
ssize_t safewrite(int fd, char* buf, size_t count);
int setWriteLock(int fd, int start, int length, struct flock* lock);
int releaseWriteLock(int fd, struct flock* lock);
int openFile(const char* path, int* fd, int flags, mode_t mode);
int closeFile(int fd);
int safemalloc(char** buf, int length);
int createPipe(int* readfd, int* writefd);
int readFromFifo(int fifo, char* buf, int size);
int writeToFifo(int fifo, char* buf, int size);
off_t fileSize(int fd);
int bind_inet_socket(uint16_t port,int type);
int make_socket(int domain, int type);
int addClient(int sfd);
void safeSleep(int seconds);

typedef struct
{
	int id;
	int fd;
	char* name;
	int* playedGames;
	int isActive;
	pthread_mutex_t *mutex;
} player;

typedef struct
{
	int id;
	player* allPlayers;

} thread_info;

typedef struct
{
	thread_info* tinfo;
	int playerOne;
	int playerTwo;
	
}game_thread_info;
