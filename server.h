#include "library.h"

void usage();
void siginthandler(int sig);
void clean(pthread_mutex_t *mutex);
void *GameThreadFunction(void* arg);
void GetRandomNumbers(int* numbers);
void *playerThreadFunction(void *arg);
void SendRankingToAll(player* players);
void clean(pthread_mutex_t *playerMutex);
void GetWord(char* slowo, int lineNumber);
int CheckIfAllGamesPlayed(player* players);
int CheckWord(char* slowo, char* odpowiedz);
void updatePlayerDirection(thread_info targ);
int PlayerDisconnected(player* plr, int ind);
void sendWordToPlayer(int fd, const char* msg);
void SendRankingToPlayer(int ind, player* players);
void startGameThread(game_thread_info* gtinfos, int ind);
void clean_structures(player* players);
void WaitForPlayers(pthread_mutex_t *m1, pthread_mutex_t *m2);
void CheckPossibleGames(thread_info *tinfo, int currentPlayersNumer);
void doWork(int socket, pthread_t *thread, thread_info *targ, int* currentThreadNumber);
void EndGame(int p1score, int p2score, int playerOne, int playerTwo, thread_info* tinfo );
void startPlayerThread(int fd, pthread_t *thread, thread_info *tinfo, int* currentThreadNumber);
void init(pthread_t *thread, thread_info *tinfo, pthread_mutex_t *playerMutexes, player *players);








