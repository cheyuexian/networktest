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
#include <thread>
#include <vector>
void sig(int sig)
{
	printf("sig------------%d\n", sig);
}
#include <vector>
using namespace std;
int suc = 0;

int main(int argc,char** argv)
{
	int n = 1;
	vector<thread> ts;
	if (argc > 1) {
		n = stoi(argv[1]);
	}
	
	//	if (bindaddr) inet_aton(bindaddr, &sa.sin_addr);

	/*if (bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
		printf("bind error\n");

		close(s);
	}
	
	printf("bind success!\n");*/
	
	signal(SIGPIPE, sig);
	while (n--) {
		ts.emplace_back(thread([]() {
			int s, on = 1;
			struct sockaddr_in sa;
			short port = 8888;
			if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				printf("socket error\n");
			}

			sa.sin_family = AF_INET;
			sa.sin_port = htons(port);
			sa.sin_addr.s_addr = inet_addr("127.0.0.1");
			int maxfd = 0;
			int r = connect(s, (sockaddr*)(&sa), sizeof(sockaddr));
			if (r < 0) {
				printf("connect fail\n");
			}
			else {
			//	printf("connect success\n");

				//printf("connect success %d\n", suc++);
			}
			//usleep(1000);
		}));
	}

	for (auto &t : ts) {
		t.join();
	}
	printf("end\n");
	
	getchar();
	return 0;
}