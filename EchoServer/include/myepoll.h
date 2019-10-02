#ifndef MYEPOLL_H
#define MYEPOLL_H

#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int Epoll_create(int size);
int Epoll_ctl(int epfd, int op, int fd, struct epoll_event *epoll_event);
int SetNonBlock(int fd);
void Epoll_ctl_add_fd(int epfd, int fd, uint32_t op);
void Epoll_ctl_mod_fd(int epfd, int fd, uint32_t op);

#endif
 
