#include "library.h"

void siginthandler(int sig);
void usage();
void *playerThreadFunction(void *arg);
void *ordersThreadFunction(void *arg);
void *moveThreadFunction(void *arg);
void init(pthread_t *thread, thread_info *targ, pthread_mutex_t *playerMutex, player *players);
void clean(pthread_mutex_t *playerMutex);
void doWork(int socket, pthread_t *thread, thread_info *targ, int* currentThreadNumber);
void startFreePlayerThread(int fd, pthread_t *thread, thread_info *targ, int* currentThreadNumber);
void updatePlayerDirection(thread_info targ);
void *GameThreadFunction(void* arg);
