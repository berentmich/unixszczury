#include "library.h"

void siginthandler(int sig);
void usage();
void clean(pthread_mutex_t *playerMutex);
void doWork(int socket, pthread_t *thread, thread_info *targ, int* currentThreadNumber);
void updatePlayerDirection(thread_info targ);
void *GameThreadFunction(void* arg);
void CheckPossibleGames(thread_info *tinfo, int currentPlayersNumer);
void startGameThread(game_thread_info* gtinfos, int ind);
void startPlayerThread(int fd, pthread_t *thread, thread_info *tinfo, int* currentThreadNumber);
void init(pthread_t *thread, thread_info *tinfo, pthread_mutex_t *playerMutexes, player *players);
void *playerThreadFunction(void *arg);
void sendWordToPlayer(int fd, const char* msg);
void GetRandomNumbers(int* numbers);
int CheckWord(char* slowo, char* odpowiedz);
int PlayerDisconnected(player* plr, int ind);
void SendRankingToPlayer(int ind, player* players);
void SendRankingToAll(player* players);
int CheckIfAllGamesPlayed(player* players);
void WaitForPlayers(pthread_mutex_t *m1, pthread_mutex_t *m2);
void GetWord(char* slowo, int lineNumber);
void clean(pthread_mutex_t *mutex);
