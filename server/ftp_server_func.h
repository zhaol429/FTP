#ifndef _FTP_SERVER_FUNC_H_
#define _FTP_SERVER_FUNC_H_

#include <mysql/mysql.h>
#include <crypt.h>
#include <shadow.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <sys/epoll.h>

#define CLIENTBLOG "client_blog.txt"
typedef struct childdata{
	pid_t pid;
	int fdw;		//
	short busy;     //0代表子进程忙碌,1代表非忙碌
}cda, *pcda;

typedef struct traindata{
	int len;        //代表其后的长度
	char buf[1000]; //真实数据长度
}tda, *ptda;

//子进程接收new_sfd并向客户端发送文件
void Makechild(pcda, int);
void handle_request(int);
void Recvfd(int, int *, struct sockaddr_in *);
int Sendfilesize(int, int);
void Sendfile(int, char *, long);
void Sendn(int, char *, int);

//主进程发送new_sfd给子进程
int Tcpinit(char *, char *);
void Eventinit(struct epoll_event *, int);
void Sendfd(int, int, struct sockaddr_in);


void Recvn(int, char *, int);
void Lsfun(char *, int);
void Getfiletype(int *, char *);
void Getrwx(int *, char *);
void Removefun(char[], int);
void Getsfun(char[], int, long);
void Pwdfun(int);
void Cdfun(char *, int);
void Putsfun(char[], int);

void Clientconnectblog(char *, int, char *, int);
int insertBlog(char *, int, char *, char *, int);

#endif
