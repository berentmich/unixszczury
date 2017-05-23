#include "server.h"

volatile sig_atomic_t stop = 0;

int main(int argc, char** argv)
{
	if(argc !=2) {
		usage();
		exit(EXIT_SUCCESS);
	}
	int socket;

	pthread_t player_threads[MAX_PLAYERS];
	pthread_mutex_t playerMutexes[MAX_PLAYERS];
	thread_info tinfo[MAX_PLAYERS];
	int currentPlayersNumber =0;
	player players[MAX_PLAYERS];
	socket = bind_inet_socket(atoi(argv[1]), SOCK_STREAM);
	init(player_threads, tinfo, playerMutexes, players);
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
	 i = currentPlayersNumer - 1;
	 game_thread_info gtinfos[i-1];
		for(j = 0; j < i; j++){
				
				gtinfos[j].tinfo = tinfo;
				gtinfos[j].playerOne = i;
				gtinfos[j].playerTwo = j;
					startGameThread(gtinfos, j);
					printf("%d %d \n",i, j);
			}
}

void startGameThread(game_thread_info* gtinfos, int ind){
	pthread_t t;
	if (pthread_create(&t, NULL, GameThreadFunction, (void *)&gtinfos[ind]) != 0)
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
		char slowo[NORMAL_MSG_SIZE];
		int fdp1 = tinfo->allPlayers[gtinfo.playerOne].fd;
		int fdp2 = tinfo->allPlayers[gtinfo.playerTwo].fd;
		
		while(1){
			if (pthread_mutex_lock(tinfo->allPlayers[gtinfo.playerOne].mutex) != 0)
				error("pthread_mutex_lock");
			if (pthread_mutex_trylock(tinfo->allPlayers[gtinfo.playerTwo].mutex) == EBUSY){
				pthread_mutex_unlock(tinfo->allPlayers[gtinfo.playerOne].mutex);
			}
			else{
				break;
			}
		}
		printf("gra players: %d %d rozpoczeta\n", gtinfo.playerOne, gtinfo.playerTwo);
		FD_ZERO(&rfds);
		FD_SET(fdp1, &rfds);
		FD_SET(fdp2, &rfds);
		
		sendWordToPlayer(fdp1, "Gra rozpoczeta\n");
		sendWordToPlayer(fdp2, "Gra rozpoczeta\n");
		int maxFd = fdp2 > fdp1 ? fdp2 : fdp1;
		
		while(1){
			/**/memset(&buffer[0], 0, sizeof(buffer));
			memset(&slowo[0], 0, sizeof(slowo));
			snprintf(slowo, NORMAL_MSG_SIZE, "sloweczko\n");
			int rv= select(maxFd + 1, &rfds, NULL, NULL, NULL);
			
			if(rv > 0){
				printf("max%d fd2 %d fd1 %d\n", maxFd, fdp2, fdp1);	
				if(FD_ISSET(fdp1, &rfds)){
					//CheckWord();
					if((size = TEMP_FAILURE_RETRY(recv(fdp1, buffer, NORMAL_MSG_SIZE, 0))) >= 0){
						/**/printf("buffer %s \n", buffer);
						printf("slowo %s \n", slowo);
						printf("roznica %d \n", strcmp(buffer, slowo));
						printf("halko\n");
						if(strcmp(buffer, slowo) == 0){
							printf("wygral");
							break;
						}
							
					}

				}
				
				if(FD_ISSET(fdp2, &rfds)){
					//CheckWord();
					if((size = TEMP_FAILURE_RETRY(recv(fdp2, buffer, NORMAL_MSG_SIZE, 0))) >= 0){
						printf("halko\n");
						printf("buffer %s \n", buffer);
						printf("slowo %s \n", slowo);
						printf("roznica %d \n", strcmp(buffer, slowo));
						printf("halko\n");
						if(strcmp(buffer, slowo) == 0){
							printf("wygral");

							break;
						}
							
					}

				}
			
			}
				FD_SET(fdp1, &rfds);
				FD_SET(fdp2, &rfds);
		}
		sendWordToPlayer(fdp1, "Gra koniec\n");
		sendWordToPlayer(fdp2, "Gra koniec\n");
		pthread_mutex_unlock(tinfo->allPlayers[gtinfo.playerOne].mutex);
		pthread_mutex_unlock(tinfo->allPlayers[gtinfo.playerTwo].mutex);

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

void init(pthread_t *thread, thread_info *tinfo, pthread_mutex_t *playerMutexes, player *players) {
	int i;

	for (i = 0; i < MAX_PLAYERS; i++) {
		players[i].id = -1;
		players[i].fd = -1;
		pthread_mutex_init(&playerMutexes[i], NULL);
		players[i].mutex  = &playerMutexes[i];
	}
	for (i = 0; i < MAX_PLAYERS; i++) {
		tinfo[i].id = i;
		
		tinfo[i].allPlayers = players;
	}
}

void *playerThreadFunction(void *arg) {
	
	thread_info tinfo;
	int nameAvailable;
	memcpy(&tinfo, arg, sizeof(tinfo));

	fd_set rfds;
	while (!stop) {//rejestracja
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
	
	return NULL;
}


void clean(pthread_mutex_t *mutex) {
	if (pthread_mutex_destroy(mutex) != 0)
		error("pthread_mutex_destroy");
}

void sendWordToPlayer(int fd, const char* msg) {
	char message[NORMAL_MSG_SIZE];
	memset(&message[0], 0, sizeof(message));
	snprintf(message, NORMAL_MSG_SIZE, msg);
	if (TEMP_FAILURE_RETRY(send(fd, message, NORMAL_MSG_SIZE, 0)) < 0)
		error("send");
}
