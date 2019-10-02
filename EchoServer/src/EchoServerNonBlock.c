#include "config.h"

int main(int argc, char* argv[])
{
    int iListenfd, iConnfd, iEpfd, iFd, iFdNum, iNum, n;
    struct sockaddr_in slCliAddr;
    socklen_t slCliAddrLen;
    struct epoll_event slEvents[MAX_EPOLL_EVENTS_SIZE];
    char Buffer[MAX_BUFFER_SIZE];
    char *pIn, *pOut;
    pIn = pOut = Buffer;

    if (argc != 2) {
      printf("Usage: ./EchoServerNonBlock port\n");
    }
    iListenfd = TcpIpv4ServerInit((uint16_t)atoi(argv[1]));
    iEpfd = Epoll_create(MAX_EPOLL_EVENTS_SIZE);
    Epoll_ctl_add_fd(iEpfd, iListenfd, EPOLLIN);
    for (;;) {
        if ((iFdNum = epoll_wait(iEpfd, slEvents, MAX_EPOLL_EVENTS_SIZE, -1)) < 0) {
          if (errno == EINTR) {
            continue;
          }
          perror("epoll_wait");
          exit(1);
        } else if (iFdNum == 0) {
          continue;
        }
        for (int i = 0; i < iFdNum; i++) {
            if (slEvents[i].data.fd == iListenfd) {
                slCliAddrLen = sizeof(slCliAddr);
                iConnfd = Accept(iListenfd, (SA *)&slCliAddr, &slCliAddrLen);
                Epoll_ctl_add_fd(iEpfd, iConnfd, EPOLLIN);
            } else {
                iFd = slEvents[i].data.fd;
                if (slEvents[i].events & EPOLLIN) {
                  if ((iNum = read(iFd, pIn, (&Buffer[MAX_BUFFER_SIZE] - pIn))) < 0) {
                      if (errno == ECONNRESET) {
                          Epoll_ctl(iEpfd, EPOLL_CTL_DEL, iFd, slEvents);
                          Close(iFd);
                      } else if (errno != EWOULDBLOCK) {
                          perror("read");
                          exit(1);
                      } 
                  } else if (iNum == 0) {
                      Epoll_ctl(iEpfd, EPOLL_CTL_DEL, iFd, slEvents);
                      Close(iFd);
                  } else {
                      pIn += iNum;
                      Epoll_ctl_mod_fd(iEpfd, iFd, EPOLLOUT);
                  }
                }
                if ((slEvents[i].events & EPOLLOUT) && ((n = pIn - pOut) > 0)) {
                  if ((iNum = write(iFd, pOut, n)) < 0) {
                      if (errno != EWOULDBLOCK) {
                          perror("write");
                          exit(1);
                      }
                  } else {
                      pOut += iNum;
                      if (pIn == pOut) {
                          pIn = pOut = Buffer;
                      }
                      Epoll_ctl_mod_fd(iEpfd, iFd, EPOLLIN);
                  }
                }
            }
        }
    }
    Close(iEpfd);
    return 0;
}

int TcpIpv4ServerInit(uint16_t port)
{
    int iSockfd;
    struct sockaddr_in slServerAddr;
    iSockfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&slServerAddr, sizeof(slServerAddr));
    slServerAddr.sin_family = AF_INET;
    slServerAddr.sin_port = htons(port);
    slServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(iSockfd, (SA *)&slServerAddr, sizeof(slServerAddr));
    
    Listen(iSockfd, LISTENQ);

    return iSockfd;
}

