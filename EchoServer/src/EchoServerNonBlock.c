#include "config.h"

typedef struct SLClient{
  int iSockfd;
  char *Buffer;
  char *pIn;
  char *pOut;
}SLClient;

int main(int argc, char* argv[])
{
    int iListenfd, iConnfd, iEpfd, iFd, iFdNum, iNum;
    struct sockaddr_in slCliAddr;
    socklen_t slCliAddrLen;
    struct epoll_event slEvents[MAX_EPOLL_EVENTS_SIZE], slEvent;

    if (argc != 2) {
      printf("Usage:./%s port\n", argv[0]);
    }
    iListenfd = TcpIpv4ServerInit((uint16_t)atoi(argv[1]));
    iEpfd = Epoll_create(MAX_EPOLL_EVENTS_SIZE);
    SLClient* slListener = NULL;
    if ((slListener = (SLClient *)malloc(sizeof(SLClient))) == NULL) {
      perror("malloc");
      exit(1);
    }
    slListener->iSockfd = iListenfd;
    slListener->Buffer = NULL;
    slListener->pIn = NULL;
    slListener->pOut = NULL;
    slEvent.data.ptr = slListener;
    slEvent.events = EPOLLIN;
    Epoll_ctl(iEpfd, EPOLL_CTL_ADD, slListener->iSockfd, &slEvent);
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
            SLClient* slClientCurrent = (SLClient *)slEvents[i].data.ptr;
            int iFd = slClientCurrent->iSockfd;
            if (iFd == iListenfd) {
                slCliAddrLen = sizeof(slCliAddr);
                if ((iConnfd = accept(iListenfd, (SA *)&slCliAddr, &slCliAddrLen)) > 0) {
                  SLClient* slClient = NULL;
                  if ((slClient = (SLClient *)malloc(sizeof(SLClient))) == NULL) {
                    perror("malloc");
                    exit(1);
                  }
                  slClient->iSockfd = iConnfd;
                  if ((slClient->Buffer = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE)) == 0) {
                    perror("malloc");
                    exit(1);
                  }
                  slClient->pIn = slClient->Buffer;
                  slClient->pOut = slClient->Buffer;
                  slEvent.data.ptr = slClient;
                  slEvent.events = EPOLLIN;
                  Epoll_ctl(iEpfd, EPOLL_CTL_ADD, slClient->iSockfd, &slEvent);
                }
                if (iConnfd < 0) {
                  if (errno != EINTR && errno != ECONNABORTED) {
                    perror("accept");
                    exit(1);
                  }
                }
                continue;
            } else {
                if (slEvents[i].events & EPOLLIN) {
                  if ((iNum = read(iFd, slClientCurrent->pIn, (size_t)(slClientCurrent->Buffer + MAX_BUFFER_SIZE - slClientCurrent->pIn))) < 0) {
                      if (errno == ECONNRESET) {
                          Epoll_ctl(iEpfd, EPOLL_CTL_DEL, iFd, slEvents);
                          Close(iFd);
                          free(slClientCurrent->Buffer);
                          free(slClientCurrent);
                      } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
                          continue;
                      } else {
                          perror("read");
                          exit(1);
                      }
                  } else if (iNum == 0) {
                      Epoll_ctl(iEpfd, EPOLL_CTL_DEL, iFd, slEvents);
                      Close(iFd);
                      free(slClientCurrent->Buffer);
                      free(slClientCurrent);
                  } else {
                      slClientCurrent->pIn += iNum;
                      slEvent.data.ptr = slClientCurrent;
                      slEvent.events = EPOLLOUT;
                      Epoll_ctl(iEpfd, EPOLL_CTL_MOD, iFd, &slEvent);
                  }
                }
                if (slEvents[i].events & EPOLLOUT) {
                  if ((iNum = write(iFd, slClientCurrent->pOut, (size_t)(slClientCurrent->pIn - slClientCurrent->pOut))) < 0) {
                      if (errno == EWOULDBLOCK || errno == EAGAIN) {
                        continue;
                      } else {
                          perror("write");
                          exit(1);
                      }
                  } else {
                      slClientCurrent->pOut += iNum;
                      if (slClientCurrent->pIn == slClientCurrent->pOut) {
                          slClientCurrent->pIn = slClientCurrent->Buffer;
                          slClientCurrent->pOut = slClientCurrent->Buffer;
                      }
                      slEvent.data.ptr = slClientCurrent;
                      slEvent.events = EPOLLIN;
                      Epoll_ctl(iEpfd, EPOLL_CTL_MOD, iFd, &slEvent);
                  }
                }
            }
        }
    }
    free(slListener);
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
    
    int iOpt;
    if (setsockopt(iSockfd, SOL_SOCKET, SO_REUSEADDR, &iOpt, sizeof(iOpt)) < 0) {
      perror("setsockopt:reuseaddr");
    }
    Bind(iSockfd, (SA *)&slServerAddr, sizeof(slServerAddr));
    
    Listen(iSockfd, LISTENQ);

    return iSockfd;
}

