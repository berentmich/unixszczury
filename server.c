#include "server.h"

volatile sig_atomic_t stop = 0;
static const char filename[] = "./slowa";

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
	srand(14334);
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
		CheckPossibleGames(tinfo, *currentPlayersNumber);
		
	}
}

void CheckPossibleGames(thread_info *tinfo, int currentPlayersNumer){
	printf("currentplayers: %d\n", currentPlayersNumer);
	int i, j;
	 i = currentPlayersNumer - 1;
	 game_thread_info gtinfos[i-1];
		for(j = 0; j < i; j++){
			printf("player %d is %d\n", j,tinfo->allPlayers[j].isActive );
				if(tinfo->allPlayers[j].isActive != -1){
					gtinfos[j].tinfo = tinfo;
					gtinfos[j].playerOne = i;
					gtinfos[j].playerTwo = j;
					startGameThread(gtinfos, j);
				}
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
		char slowo1[NORMAL_MSG_SIZE];
		char slowo2[NORMAL_MSG_SIZE];
		int word_numbers[GAME_LENGTH];
		int fdp1 = tinfo->allPlayers[gtinfo.playerOne].fd;
		int fdp2 = tinfo->allPlayers[gtinfo.playerTwo].fd;
		int currSlowo1 = 0, nextSlowo1 = 0,currSlowo2 = 0, nextSlowo2 = 0, p1score =0, p2score =0;
		WaitForPlayers(tinfo->allPlayers[gtinfo.playerOne].mutex, tinfo->allPlayers[gtinfo.playerTwo].mutex);
		if(tinfo->allPlayers[gtinfo.playerOne].isActive == -1 || tinfo->allPlayers[gtinfo.playerTwo].isActive == -1){
			
			pthread_mutex_unlock(tinfo->allPlayers[gtinfo.playerOne].mutex);
			pthread_mutex_unlock(tinfo->allPlayers[gtinfo.playerTwo].mutex);
			return;
		}
			
		tinfo->allPlayers[gtinfo.playerOne].isPlaying = 1;
		tinfo->allPlayers[gtinfo.playerTwo].isPlaying = 1;
		printf("gra players: %d %d rozpoczeta\n", gtinfo.playerOne, gtinfo.playerTwo);
		GetRandomNumbers(word_numbers);
		
		FD_ZERO(&rfds);
		FD_SET(fdp1, &rfds);
		FD_SET(fdp2, &rfds);
		GetWord(slowo1, word_numbers[currSlowo1]);
		GetWord(slowo2, word_numbers[currSlowo2]);
		sendWordToPlayer(fdp1, "Gra rozpoczeta\n");
		sendWordToPlayer(fdp2, "Gra rozpoczeta\n");
		sendWordToPlayer(fdp1,slowo1);
		sendWordToPlayer(fdp2,slowo2);
		int maxFd = fdp2 > fdp1 ? fdp2 : fdp1;
		
		while(1){
			if(currSlowo1 < GAME_LENGTH && currSlowo2 <GAME_LENGTH){
				if(nextSlowo1){
					GetWord(slowo1, word_numbers[currSlowo1]);
					sendWordToPlayer(fdp1,slowo1);
					nextSlowo1 = 0;
				}
				if(nextSlowo2){
					GetWord(slowo2, word_numbers[currSlowo2]);
					sendWordToPlayer(fdp2,slowo2);
					nextSlowo2 = 0;
				}
				int rv= select(maxFd + 1, &rfds, NULL, NULL, NULL);		
				memset(&buffer[0], 0, sizeof(buffer));

				if(rv > 0){
					if(FD_ISSET(fdp1, &rfds)){
						//CheckWord();
						if((size = TEMP_FAILURE_RETRY(recv(fdp1, buffer, NORMAL_MSG_SIZE, 0))) >= 0){
													
							if(strlen(buffer) == 0){
								if(PlayerDisconnected(tinfo->allPlayers, gtinfo.playerOne)){
									p1score = -1;

								}
								break;
							}
								
							if(nextSlowo1 = CheckWord(slowo1, buffer)){
									p1score++;
									currSlowo1++;
									sendWordToPlayer(fdp1, "+1\n");
							}
						}

					}
				
					if(FD_ISSET(fdp2, &rfds)){
						//CheckWord();
						if((size = TEMP_FAILURE_RETRY(recv(fdp2, buffer, NORMAL_MSG_SIZE, 0))) >= 0){
							if(strlen(buffer) ==0){
								if(PlayerDisconnected(tinfo->allPlayers, gtinfo.playerTwo)){
									p2score = -1;
								}
								break;
							}
							if(nextSlowo2 = CheckWord(slowo2, buffer)){
									p2score++;
									currSlowo2++;
									sendWordToPlayer(fdp2, "+1\n");
							}						
						}

					}
			
				}
				FD_SET(fdp1, &rfds);
				FD_SET(fdp2, &rfds);
			}
			else
				break;
		}
		if(p1score > p2score){
			sendWordToPlayer(fdp1, "Wygrales\n");
			sendWordToPlayer(fdp2, "Przegrales\n");
			tinfo->allPlayers[gtinfo.playerOne].playedGames[gtinfo.playerTwo] = 1;
			tinfo->allPlayers[gtinfo.playerTwo].playedGames[gtinfo.playerOne] = 0;
		}
		else{
			sendWordToPlayer(fdp1, "Przegrales\n");
			sendWordToPlayer(fdp2, "Wygrales\n");
			tinfo->allPlayers[gtinfo.playerOne].playedGames[gtinfo.playerTwo] = 0;
			tinfo->allPlayers[gtinfo.playerTwo].playedGames[gtinfo.playerOne] = 1;
		}
		tinfo->allPlayers[gtinfo.playerOne].isPlaying = 0;
		tinfo->allPlayers[gtinfo.playerTwo].isPlaying = 0;

		pthread_mutex_unlock(tinfo->allPlayers[gtinfo.playerOne].mutex);
		pthread_mutex_unlock(tinfo->allPlayers[gtinfo.playerTwo].mutex);


}
void GetRandomNumbers(int* numbers){
	int i, r;
	for(i=0; i< GAME_LENGTH; i++){
		r = rand() % FILE_LENGTH;
		numbers[i] = r;

	}

}
void GetWord(char* slowo, int lineNumber){
	memset(&slowo[0], 0, sizeof(slowo));
	FILE *file = fopen(filename, "r");
	int count = 0;
	if ( file != NULL ){
		char line[20];
		while (fgets(line, sizeof line, file) != NULL) 
		{
			if (count == lineNumber)
			{
				strcpy(slowo, line);
				break;
			}
			else
			{
				count++;
			}
		}
		fclose(file);
	}
	else
	{
		error("File erorr");
	}
}

void startPlayerThread(int fd, pthread_t *thread, thread_info *tinfo, int* currentThreadNumber) {
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (tinfo -> allPlayers[i].id == -1) {
			tinfo -> allPlayers[i].id = i;
			tinfo -> allPlayers[i].fd = fd;
			tinfo -> allPlayers[i].isActive = 1;

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
		players[i].isPlaying = 0;
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
	int nameAvailable, fd;
	memcpy(&tinfo, arg, sizeof(tinfo));
	fd = tinfo.allPlayers[tinfo.id].fd;
	fd_set rfds;
	size_t size;
	char buffer[NORMAL_MSG_SIZE];
	while (!stop) {//rejestracja
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		int rv= select(fd + 1, &rfds, NULL, NULL, NULL);
		if (rv  > 0 ) { 
			if(!(tinfo.allPlayers[tinfo.id].isPlaying)){
				memset(&buffer[0], 0, sizeof(buffer));
				if((size = TEMP_FAILURE_RETRY(recv(fd, buffer, NORMAL_MSG_SIZE, 0))) >= 0){
					if(strlen(buffer) == 0 || tinfo.allPlayers[tinfo.id].isActive ==-1){
						PlayerDisconnected(tinfo.allPlayers,tinfo.id);
						break;
					}
			}		
				}
			
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
void WaitForPlayers(pthread_mutex_t *m1, pthread_mutex_t *m2){
		while(1){
			if (pthread_mutex_lock(m1) != 0)
				error("pthread_mutex_lock");
			if (pthread_mutex_trylock(m2) == EBUSY){
				pthread_mutex_unlock(m1);
			}
			else{
				break;
			}
		}
}

int CheckWord(char* slowo, char* odpowiedz){
	if(strncmp(odpowiedz, slowo, strlen(slowo)-1) == 0){
		return 1;
	}
	return 0;
}
int PlayerDisconnected(player* plr, int ind){

	plr[ind].isActive = -1;

	printf("player %d disconected hisActive %d\n", plr[ind].id, plr[ind].isActive );
	return plr[ind].isPlaying;
}
