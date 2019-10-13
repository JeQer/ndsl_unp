#include "config.h"

int main(int argc, char **argv)
{
	struct sockaddr_in serv_addr, cli_addr;
	int sockfd, connfd;
	socklen_t cli_len;
	pid_t child_pid;

  if (argc != 2){
    printf("Usage:./EchoServerFork port\n");
  }
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons((uint16_t)atoi(argv[1]));
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(sockfd, (SA *) & serv_addr, sizeof(serv_addr));

	Listen(sockfd, LISTENQ);
  
  Signal(SIGCHLD, my_sig_chld);

	for (;;) {
		cli_len = sizeof(cli_addr);
		connfd = Accept(sockfd, (SA *) & cli_addr, &cli_len);

		if ((child_pid = Fork()) == 0) {
			Close(sockfd);
			my_str_serv(connfd);
			exit(0);
		}
		Close(connfd);
	}
  
  return 0;
}

void my_sig_chld(int singo)
{
  pid_t pid;
  int stat;
  while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
    printf("Server's %d child terminated\n",pid);
  }
  return ;
}

void my_str_serv(int sockfd)
{
  ssize_t n;
  char buf[MAXLINE];
  for (;;) {
    while ((n = read(sockfd, buf, MAXLINE)) > 0) {
      Writen(sockfd, buf, n);
    }
    if (n < 0 && errno == EINTR) {
      continue;
    } else if (n < 0) {
      perror("my_str_serv:read");
      exit(1);
    }
    return;
  }
}
