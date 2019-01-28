#include <cstdio>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
void sig(int sig)
{
	printf("sig------------%d\n", sig);
}
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
	printf("hello from select!\n");
	int maxfd = 0;
	signal(SIGPIPE, sig);
	vector<int> fs;
	fs.push_back(s);
	fd_set rfds, wfds, efds;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);
	//FD_SET(s, &rfds);
	int sum = 0;
	while (1)
	{
		struct timeval  tv;
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		int max = -1;

		FD_ZERO(&rfds);
		FD_ZERO(&wfds);

		for (auto &f : fs) {
			FD_SET(f, &rfds);

			if (f > max) {
				max = f;
			}
		}

		int res = select(max + 1, &rfds, &wfds, nullptr, &tv);
		if (res > 0)
		{
			for (int f : fs)
			{
				printf("f=%d\n", f);
				if (FD_ISSET(f, &rfds)) {
					if (f == s) {
						struct sockaddr_in  client_addr;
						socklen_t sin_size = sizeof(client_addr);
						int new_fd = -1;

						if ((new_fd = accept(s, (struct sockaddr *)&client_addr, &sin_size)) == -1)
						{
							perror("accept");
							exit(1);
						}
						else
						{
							FD_SET(new_fd, &rfds);
							fs.push_back(new_fd);
							printf("accept %d\n", sum++);
							break;
						}
					}
					else {
						char buff[256] = { 0 };
						int r = read(f, buff, sizeof(buff));
						if (r > 0)
						{
							printf("%d read %s\n", f, buff);
							time_t tks = time(NULL);
							char* ch = ctime(&tks);
							printf("p=%p\n", ch);
							int len = send(f, ch, strlen(ch), 0);
							printf("sent len=%d\n", len);
							/*free(ch);
							ch = NULL;*/
						}
						else if (r == 0) {
							close(f);
							for (auto i = fs.begin(); i != fs.end(); i++) {
								if (*i == f) {
									fs.erase(i);
									printf("erase\n");
									break;
								}
							}

							sum--;
							break;
						}
					}
				}
			}

			/*if (FD_ISSET(new_fd, &rfds))
			{
				printf("new_fd read\n");


			}*/
			//	if (FD_ISSET(new_fd, &wfds))
			//	{
			//		printf("new_fd is set\n");
			//		/*char buff[] = "1234";
			//		sleep(1);
			//		int len = send(new_fd, buff, strlen(buff), 0);
			//		printf("sent len=%d\n", len);*/
			//	}
			//}
		}
	}
	return 0;
}