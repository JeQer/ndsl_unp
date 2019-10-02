#include "myepoll.h"

int Epoll_create(int size)
{
    int iRes;
    if ((iRes = epoll_create(size)) < 0) {
		perror("epoll_create");
		exit(1);
	}
    return iRes;
}
int Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    if (epoll_ctl(epfd, op, fd, event) < 0) {
		perror("epoll_ctl");
		exit(1);
	}
    return 0;
}
int SetNonBlock(int fd)
{
    int iFl;
    if ((iFl = fcntl(fd, F_GETFL, 0)) < 0) {
        perror("fcntl:F_GETFL");
        return -1;
    }
    if (fcntl(fd, F_SETFL, iFl | O_NONBLOCK) < 0){
        perror("fcntl:F_SETFL");
        return -1;
    }
    return 0;
}
void Epoll_ctl_add_fd(int epfd, int fd, uint32_t op)
{
  struct epoll_event event;
  SetNonBlock(fd);
  event.data.fd = fd;
	event.events = op;
	Epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}
void Epoll_ctl_mod_fd(int epfd, int fd, uint32_t op)
{
  struct epoll_event event;
  event.data.fd = fd;
	event.events = op;
	Epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
}
