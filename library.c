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

ssize_t saferead(int fd, char* buf, size_t count)
{
	int c;
	size_t len = 0;
	do
	{
		c = TEMP_FAILURE_RETRY(read(fd, buf, count));
		if(c < 0) return c;
		if(c == 0)return len;
		buf += c;
		len += c;
		count -= c;
	} while(count > 0);
	return len;
}

ssize_t safewrite(int fd, char* buf, size_t count)
{
	int c;
	size_t len = 0;
	do
	{
		c = TEMP_FAILURE_RETRY(write(fd, buf, count));
		if(c < 0) return c;
		count -= c;
		len += c;
		buf += c;
	} while(count > 0);
	return len;
}

int setWriteLock(int fd, int start, int length, struct flock* lock)
{
	if(lock == NULL) return -1;
	memset(lock, 0, sizeof(struct flock));
	lock -> l_type = F_WRLCK;
	lock -> l_start = start;
	lock -> l_whence = SEEK_SET;
	lock -> l_len = length;
	if(TEMP_FAILURE_RETRY(fcntl(fd, F_SETLKW, lock)) == -1)
		return -1;
	return 0;
}

int releaseWriteLock(int fd, struct flock* lock)
{
	if(lock == NULL) return -1;
	lock -> l_type = F_UNLCK;
	if(TEMP_FAILURE_RETRY(fcntl(fd, F_SETLKW, lock)) == -1)
		return -1;
	return 0;
}

int closeFile(int fd)
{
	if(TEMP_FAILURE_RETRY(close(fd)) < 0)
		return -1;
	return 0;
}

int openFile(const char* path, int* fd, int flags, mode_t mode)
{
	(*fd) = TEMP_FAILURE_RETRY(open(path, flags, mode));
	if((*fd) < 0)
		return -1;
	return 0;
}

int readLine(int fd, char* buf)
{
	if(buf == NULL) return -1;
	int len = 0;
	int c = 0;
	errno = 0;
	while((c = saferead(fd, buf+len, 1)) > 0 && buf[len] != '\n')
		len++;
	buf[len] = '\0';
	if(errno != 0) return -1;
	if(c == EOF) return 0;
	return len;
}

int safemalloc(char** buf, int length)
{
	(*buf) = (char*) malloc(length * sizeof(char));
	if((*buf) == NULL)
		return -1;
	return 0;
}

int createPipe(int* readfd, int* writefd)
{
	int fd[2];
	(*readfd) = -1;
	(*writefd) = -1;
	if(pipe(fd) < 0)
		return -1;
	(*readfd) = fd[0];
	(*writefd) = fd[1];
	return 0; 
}

int readFromFifo(int fifo, char* buf, int size)
{
	if(size > PIPE_BUF) return -1;
	return saferead(fifo, buf, size);
}

int writeToFifo(int fifo, char* buf, int size)
{
	if(size > PIPE_BUF) return -1;
	int len = safewrite(fifo, buf, size);
	if(len < 0 && errno == EPIPE)
		return 0;
	if(len < 0) return -1;
	return len;
}

int queRemove(int queue)
{
	if(msgctl(queue, IPC_RMID, NULL) < 0)
		return -1;
	return 0;
}

off_t fileSize(int fd) {
	off_t ret;
	if((ret=lseek(fd,0,SEEK_END))==(off_t)-1) error("lseek");
	return ret;
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

void safeSleep(int seconds) {
	struct timespec tt, t = {seconds, 0};
	for(tt = t; nanosleep(&tt, &tt); )
		if(EINTR!=errno) 
			error("nanosleep:");
}
