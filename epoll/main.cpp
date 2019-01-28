#include <cstdio>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include <error.h>
#include <signal.h>
#include <netinet/in.h>
void sig(int sig)
{
	printf("sig------------%d\n", sig);
}
#define MAXSIZE 1024*10
#define MAXBUFF 1024

#include <vector>
using namespace std;
int main()
{
	int s, on = 1;
	struct sockaddr_in sa;
	short port = 8888;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("socket error\n");
	}
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
		printf("setsockopt error\n");

		close(s);
	}
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	//	if (bindaddr) inet_aton(bindaddr, &sa.sin_addr);

	if (bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
		printf("bind error\n");

		close(s);
	}
	if (listen(s, 10) == -1) {
		printf("listen error\n");

		close(s);
	}
	printf("hello from epoll!\n");
	int maxfd = 0;
	signal(SIGPIPE, sig);

	int epfd = epoll_create(1024);
	if (epfd == -1) {
		printf("epoll_create error\n");
	}
	epoll_event ee;
	ee.data.fd = s;
	ee.events = EPOLLIN;
	if ((epoll_ctl(epfd, EPOLL_CTL_ADD, s, &ee) == -1)) {
		printf("epoll_ctl error\n");
	}
	int total = 0;
	while (1) {

		epoll_event events[MAXSIZE];

		int nums = epoll_wait(epfd, events, MAXSIZE, -1);
		if (nums > 0) {
			total += nums;
			printf("nums=%d total=%d\n", nums, total);

			for (int i = 0; i < nums; i++) {
				epoll_event* e = events + i;
				int op = e->events;
				int fd = e->data.fd;
			//	printf("fd=%d op=%d\n", fd, op);
				if (op == EPOLLIN) {
					if (fd == s) {
						int clientFd;
						struct sockaddr_in sa;
						unsigned int saLen;
						while (1) {
							saLen = sizeof(sa);
							clientFd = accept(fd, (struct sockaddr*)&sa, &saLen);

							if (clientFd == -1) {
								if (errno == EINTR)
									continue;
								else {
									printf("accept error \n");
									break;
								}
							}
							//printf("accept fd=%d,total=%d\n", clientFd,total++);
							epoll_event ee;
							ee.data.fd = clientFd;
							ee.events = EPOLLIN;
							if ((epoll_ctl(epfd, EPOLL_CTL_ADD, clientFd, &ee) == -1)) {
								printf("epoll_ctl in error fd=%d\n", fd);
							}
							break;
						}
					}
					else {
						char buf[MAXBUFF];
						int nread;
						printf("read1\n");
						nread = read(fd, buf, MAXBUFF);
						printf("read2= %d\n",nread);

						if (nread == -1) {
							if (errno = EAGAIN) {
								printf("read eagain\n");
								nread = 0;
							}
							else {
								printf("read close\n");

								close(fd);
								continue;
							}
						}
						else if (nread == 0) {
							close(fd);
							continue;
						}
						else if (nread > 0) {
							//printf("buf=%s len=%d\n", buf, nread);

							write(fd, buf, nread);
							/*epoll_event ee;
							ee.data.fd = fd;
							ee.events = EPOLLOUT;
							if ((epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ee) == -1)) {
								printf("epoll_ctl out error  fd=%d\n", fd);
							}*/
							continue;
						}
					}
				}
				if (op == EPOLLOUT) {

				}

			}
			printf("\n");
		}
	}

}