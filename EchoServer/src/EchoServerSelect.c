#include "config.h"

int main(int argc, char **argv)
{
	struct sockaddr_in serv_addr, cli_addr;
	int sockfd, connfd, sockfd_cur;
	socklen_t cli_len;
	fd_set allset, rset;
	int i, maxfd, maxi, ready_descriptor_num;
	int client[FD_SETSIZE];
	char buf[MAXLINE];
	ssize_t n;

  if (argc != 2) {
    printf("Usage:./EchoServerSelect port\n");
  }
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons((uint16_t)atoi(argv[1]));
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(sockfd, (SA *) & serv_addr, sizeof(serv_addr));

	Listen(sockfd, LISTENQ);

	maxfd = sockfd;
	maxi = -1;
	for (i = 0; i < FD_SETSIZE; i++) {
		client[i] = -1;
	}
	FD_ZERO(&allset);
	FD_SET(sockfd, &allset);

	for (;;) {
		rset = allset;
		ready_descriptor_num =
		    Select(maxfd + 1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(sockfd, &rset)) {
			cli_len = sizeof(cli_addr);
			connfd =
			    Accept(sockfd, (SA *) & cli_addr, &cli_len);

			for (i = 0; i < FD_SETSIZE; i++) {
				if (client[i] < 0) {
					client[i] = connfd;
					break;
				}
			}
			if (i == FD_SETSIZE) {
				printf("too many clients\n");
				exit(1);
			}
			FD_SET(connfd, &allset);
			if (connfd > maxfd) {
				maxfd = connfd;
			}
			if (i > maxi) {
				maxi = i;
			}
			if ((--ready_descriptor_num) <= 0) {
				continue;
			}
		}
		for (i = 0; i <= maxi; i++) {
			if ((sockfd_cur = client[i]) < 0) {
				continue;
			}
			if (FD_ISSET(sockfd_cur, &rset)) {
				if ((n =
				     Read(sockfd_cur, buf,
					  MAXLINE)) == 0) {
					Close(sockfd_cur);
					FD_CLR(sockfd_cur, &allset);
					client[i] = -1;
				} else {
					Writen(sockfd_cur, buf, n);
				}
				if ((--ready_descriptor_num) <= 0) {
					break;
				}
			}
		}
	}
}
