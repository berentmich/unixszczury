#include "server.h"

volatile sig_atomic_t stop = 0;

int main(int argc, char** argv)
{
	if(argc !=2) {
		usage();
		exit(EXIT_SUCCESS);
	}
	int socket;

	pthread_t player_threads[MAX_THREAD];
	pthread_mutex_t playerMutex = PTHREAD_MUTEX_INITIALIZER;
	thread_info tinfo[MAX_PLAYERS];
	int currentPlayersNumber =0;
	player players[MAX_PLAYERS];
	socket = bind_inet_socket(atoi(argv[1]), SOCK_STREAM);
	init(player_threads, tinfo, &playerMutex, players);
	doWork(socket, player_threads, tinfo, &currentPlayersNumber);
/*
	for (int i = 0; i < currentThreadNumber; i++)
		if (pthread_join(thread[i], NULL) != 0)
			error("pthread_join");
	clean(&playerMutex);
	if (TEMP_FAILURE_RETRY(close(socket)) < 0)
		error("close");*/
	
    return EXIT_SUCCESS;
}

void usage() {
	fprintf(stderr,"USAGE: Program needs port\n");
}

void siginthandler(int sig) {
	stop = 1;
}

void doWork(int socket, pthread_t *thread, thread_info *tinfo, int* currentPlayersNumber) {
	fd_set base_rfds, rfds;
	int cfd;
	FD_ZERO(&base_rfds);
	FD_SET(socket, &base_rfds);
	
	while (!stop) {
		rfds = base_rfds;
		if (select(MAX_PLAYERS, &rfds, NULL, NULL, NULL) > 0) {
			if ((cfd = addClient(socket)) != -1) {
				fprintf(stderr, "Trying to add new client\n");
				if (((*currentPlayersNumber)) >= MAX_PLAYERS) { 
					fprintf(stderr, "Cannot add a new client\n");
					if (TEMP_FAILURE_RETRY(close(cfd)) < 0)
						error("close");
				} else {
					startPlayerThread(cfd, thread, tinfo, currentPlayersNumber);
				}
			}
		} else {
			if (EINTR == errno) continue;
			error("pselect");
		}
		sleep(1);
		CheckPossibleGames(tinfo, *currentPlayersNumber);
		
	}
}

void CheckPossibleGames(thread_info *tinfo, int currentPlayersNumer){
	printf("currentplayers: %d\n", currentPlayersNumer);
	int i, j;
	int p1, p2;
	 i = currentPlayersNumer - 1;
		for(j = 0; j < i; j++){
			p1 = i;
			p2 = j;
					startGameThread(tinfo, p1, p2);
					printf("%d %d \n",i, j);
			}
}

void startGameThread(thread_info *tinfo, int playerOne, int playerTwo){

	game_thread_info gtinfo;
	gtinfo.tinfo = tinfo;
	gtinfo.playerOne = playerOne;
	gtinfo.playerTwo = playerTwo;
	
	pthread_t t;
	if (pthread_create(&t, NULL, GameThreadFunction, (void *)&gtinfo) != 0)
		error("pthread_create");
	

}

void *GameThreadFunction(void *arg){


		game_thread_info gtinfo;
		memcpy(&gtinfo, arg, sizeof(gtinfo));

		thread_info* tinfo = gtinfo.tinfo;
		fd_set rfds;
		size_t size;
		char buffer[NORMAL_MSG_SIZE];
		char sendBuffer[NORMAL_MSG_SIZE];
		int fdp1 = tinfo->allPlayers[gtinfo.playerOne].fd;
		int fdp2 = tinfo->allPlayers[gtinfo.playerTwo].fd;
		printf("gra players: %d %d rozpoczeta\n", gtinfo.playerOne, gtinfo.playerTwo);
		FD_ZERO(&rfds);
		FD_SET(fdp1, &rfds);
		FD_SET(fdp2, &rfds);
		
		sendWordToPlayer(fdp1, "Gra rozpoczeta\n");
		sendWordToPlayer(fdp2, "Gra rozpoczeta\n");
		int maxFd = fdp2 > fdp1 ? fdp2 : fdp1;
			
		while(1){
			int rv= select(maxFd + 1, &rfds, NULL, NULL, NULL);
			if(rv > 0){
				if(FD_ISSET(fdp1, &rfds)){
					//CheckWord();
					if((size = TEMP_FAILURE_RETRY(recv(fdp1, buffer, NORMAL_MSG_SIZE, 0))) >= 0)
						printf("%s\n", buffer);
					
				}
				
				if(FD_ISSET(fdp2, &rfds)){
					//CheckWord();
					if((size = TEMP_FAILURE_RETRY(recv(fdp2, buffer, NORMAL_MSG_SIZE, 0))) >= 0)
						printf("%s\n", buffer);
					
				}

		}
}
}

void startPlayerThread(int fd, pthread_t *thread, thread_info *tinfo, int* currentThreadNumber) {
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (tinfo -> allPlayers[i].id == -1) {
			tinfo -> allPlayers[i].id = i;
			tinfo -> allPlayers[i].fd = fd;
			tinfo -> allPlayers[i].playedGames = (int*)malloc(MAX_PLAYERS * sizeof(int));
			for(int j = 0; j<MAX_PLAYERS; j++){
				
				tinfo->allPlayers[i].playedGames[j] = -1;
			}
				
			if (pthread_create(&thread[i], NULL, playerThreadFunction, (void *)&tinfo[i]) != 0)
				error("pthread_create");
			(*currentThreadNumber)++;
			break;
		}
	}
}

void init(pthread_t *thread, thread_info *tinfo, pthread_mutex_t *playerMutex, player *players) {
	int i;

	for (i = 0; i < MAX_PLAYERS; i++) {
		players[i].id = -1;
		players[i].fd = -1;
	}
	for (i = 0; i < MAX_PLAYERS; i++) {
		tinfo[i].id = i;
		tinfo[i].playerMutex = playerMutex;
		tinfo[i].allPlayers = players;
	}
}

void *playerThreadFunction(void *arg) {
	
	thread_info tinfo;
	int nameAvailable;
	memcpy(&tinfo, arg, sizeof(tinfo));
	if (pthread_mutex_lock(tinfo.playerMutex) != 0)
		error("pthread_mutex_lock");

	//tinfo.allPlayers[tinfo.id].id = pthread_self();
	
	pthread_mutex_unlock(tinfo.playerMutex);
	fd_set rfds;
	while (1) {//rejestracja
		FD_ZERO(&rfds);
		FD_SET(tinfo.allPlayers[tinfo.id].fd, &rfds);
		int rv= select(tinfo.allPlayers[tinfo.id].fd + 1, &rfds, NULL, NULL, NULL);

		if (rv  > 0) { // socket has something to read
			nameAvailable = 1;//IsNameAvailable(tinfo);
			if(nameAvailable)
				break;
			else
				continue;
		} else if(rv == 0) {

		} else {
			if (EINTR == errno) continue;
			error("pselect");
		}
	}
	
	while(1){//oczekiwanie na wyniki
		FD_ZERO(&rfds);
		FD_SET(tinfo.allPlayers[tinfo.id].fd, &rfds);
		int rv= select(tinfo.allPlayers[tinfo.id].fd + 1, &rfds, NULL, NULL, NULL);
		if (rv  > 0) { // socket has something to read
			//ShowRanking(tinfo);
		} else if(rv == 0) {

		} else {
			if (EINTR == errno) continue;
			error("pselect");
		}
	}
	
	return NULL;
}


void clean(pthread_mutex_t *mutex) {
	if (pthread_mutex_destroy(mutex) != 0)
		error("pthread_mutex_destroy");
}

void sendWordToPlayer(int fd, const char* msg) {
	char message[NORMAL_MSG_SIZE];
	snprintf(message, NORMAL_MSG_SIZE, msg);
	if (TEMP_FAILURE_RETRY(send(fd, message, NORMAL_MSG_SIZE, 0)) < 0)
		error("send");
}
