#include "library.h"

void error(const char* reason)
{
	fprintf(stderr, "%s", reason);
	kill(0, SIGKILL);
	exit(EXIT_FAILURE);
}

int sethandler(int sigNo, void (*handler)(int))
{
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = handler;
	if(sigaction(sigNo, &act, NULL) == -1)
		return -1;
	return 0;
}

void sigchildhandler(int sigNo)
{
	pid_t pid;
	for(;;)
	{
		pid = waitpid(0, NULL, WNOHANG);
		if(pid == 0) return;
		if(pid < 0)
		{
			if(errno == ECHILD) return;
			error("waitpid");
		}
	}
}


int bind_inet_socket(uint16_t port,int type) {
	struct sockaddr_in addr;
	int socketfd,t=1;
	socketfd = make_socket(PF_INET,type);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,&t, sizeof(t))) error("setsockopt");
	if(bind(socketfd,(struct sockaddr*) &addr,sizeof(addr)) < 0)  error("bind");
	if(SOCK_STREAM==type)
		if(listen(socketfd, BACKLOG) < 0) error("listen");
	return socketfd;
}

int make_socket(int domain, int type) {
	int sock;
	sock = socket(domain, type, 0);
	if(sock < 0) error("socket");
	return sock;
}

int addClient(int sfd)
{
	int nfd;
	if ((nfd = TEMP_FAILURE_RETRY(accept(sfd, NULL, NULL))) < 0)
	{
		if (EAGAIN == errno || EWOULDBLOCK == errno)
			return -1;
		error("accept");
	}

	return nfd;
}

