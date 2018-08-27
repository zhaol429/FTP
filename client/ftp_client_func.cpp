#include "ftp_client_func.h"
#include <unistd.h>

void Recvn(int fd, char *buf, int len)
{
	int total = 0;
	int pos;
	while (total < len)
	{
		pos = recv(fd, buf + total, len - total, 0);
		total = total + pos;
	}
}

int Tcpinit(char *ip, char *port)
{
	int ret, sfd;
	if (-1 == (sfd=socket(AF_INET, SOCK_STREAM, 0)))
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in sev; 
	bzero(&sev, sizeof(struct sockaddr_in));
	sev.sin_family = AF_INET;
	sev.sin_port = htons(atoi(port));
	sev.sin_addr.s_addr = inet_addr(ip);
	ret = connect(sfd, (struct sockaddr *)&sev, sizeof(sev));
	if (-1 == ret)
	{
		perror("connect");
		return -1;
	}
	return sfd;
}

int Loginrequest(ptda t, int sfd)
{
	int len;
	char pwd[20] = {0};
	char salt[12] = {0};
	char name[20] = {0};
	printf("user_name:");
	bzero(t, sizeof(tda));
	scanf("%s", t->buf);
	getchar();
	//getchar();
	t->len = strlen(t->buf);
	Sendn(sfd, (char *)t, 4 + t->len);
	Recvn(sfd, (char *)&len, 4);
	if (-331 == len)
	{
		printf("错误的用户名，请先注册!\n请设置用户名:");
		//getchar();
		scanf("%s", name);
		getchar();
		printf("请输入密码:");
		scanf("%s", pwd);
		getchar();
		bzero(t, sizeof(tda));
		printf("请再次输入密码:");
		scanf("%s", t->buf);
		//getchar();
		t->len = strlen(t->buf);
		if (!strcmp(pwd, t->buf))
		{
			Sendn(sfd, (char *)t, 4 + t->len);    //发送密码
			//len=strlen(name);
			bzero(t, sizeof(tda));
			t->len = strlen(name);
			strcpy(t->buf, name);
			//Sendn(sfd,(char*)&len,4);
			//Sendn(sfd,(char*)name,len);
			Sendn(sfd, (char *)t, 4 + t->len);
			printf("注册成功！\n");
			printf("welcome here!\n");
			Helpfun();
			return 0;
		}else{
			printf("密码不一致!\n你已退出!\n");
			return -1;;
		}
	}else if (331 == len){
		Recvn(sfd, (char *)&len, 4);
		Recvn(sfd, salt, len);
		printf("user_password:");
#if 0
		while (t->buf[i] = getchar())
		{
			fflush(stdin);
			if (t->buf[i] == 13)
			{
				t->buf[i] = '\0';
				break;
			}
			if (t->buf[i] != '\b')
			{
				printf("*");
				i++;
			}else{
				if (i > 0)
				{
					printf("\b \b");
					i--;
				}else{
					continue;
				}
			}
		}
#endif
		scanf("%s", pwd);
		bzero(t, sizeof(tda));
		strcpy(t->buf, crypt(pwd, salt));
		t->len = strlen(t->buf);
		Sendn(sfd, (char *)t, 4 + t->len);
		Recvn(sfd, (char *)&len, 4);
		if (230 == len)
		{
			printf("login success，welcome here!\n");
			Helpfun();
			return 0;
		}else if (-230 == len){
			printf("error passwd!\n你已退出!\n");
			return -1;;
		}
	}
}

void Eventinit(struct epoll_event *ev, int fd)
{
	bzero(ev, sizeof(struct epoll_event));
	ev->events = EPOLLIN;
	ev->events = EPOLLIN;
	ev->data.fd = fd;
}

int Sendfilesize(int sfd, int fd)
{
	struct stat buf;
	int ret = fstat(fd, &buf);
	if (-1 == ret)
	{
		 perror("fstat");
		 return -1;    //
	}
	tda t;
	printf("size=%ld Byte\n", buf.st_size);
	char bufs[20];
	bzero(bufs, sizeof(bufs));
	sprintf(bufs, "%ld\n", buf.st_size);
	t.len = strlen(bufs);
	strcpy(t.buf, bufs);
	Sendn(sfd, (char *)&t, 4 + t.len);
	return buf.st_size;
}

void Sendfile(int sfd, char filename[])
{   
	tda t;
	memset(&t, 0, sizeof(t));
	int fd; 
	if (-1 == (fd=open(filename, O_RDONLY)))
	{ 
		perror("open");
		return;
	}
	long size = Sendfilesize(sfd, fd);
	char *pmmap, *pcur;
	if (size > 100 * 1024 * 1024)
	{
		pmmap = (char *)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
		if ((char *)-1 == pmmap)
		{
			perror("mmap failed");
			return;
		}
		pcur = pmmap;
		while (pmmap + size - pcur > 1000)
		{
			bzero(&t, sizeof(tda));
			memcpy(t.buf, pcur, 1000);
			t.len = 1000;
			Sendn(sfd, (char *)&t, 4 + t.len);
			pcur += 1000;
		}
		bzero(&t, sizeof(tda));
		t.len = pmmap + size - pcur;
		memcpy(t.buf, pcur, 4 + t.len);
		Sendn(sfd, (char *)&t, 4 + t.len);
		if (-1 == munmap(pmmap, size))
		{
			perror("munmap");
			return;
		}
	}else{
		while (memset(&t, 0, sizeof(t)), (t.len=read(fd, t.buf, sizeof(t.buf))) > 0)
		{   
			Sendn(sfd, (char *)&t, 4 + t.len);
		}
	}
	t.len = -1;
	Sendn(sfd, (char *)&t.len, 4);      //发送结束符给服务器
	printf("puts success!\n");
	close(fd);
	//close(sfd);
}

void Getscontinue(ptda t, int sfd, long *total)
{
	char temp[1000];
	bzero(&temp, sizeof(t->buf));
	memcpy(temp, t->buf, sizeof(t->buf));
	char *fir, *sec, *delim = " \n";
	fir = strtok(temp, delim);
	sec = strtok(NULL, delim);
	DIR *dir;
	if (NULL == (dir=opendir(getcwd(NULL, 0))))
	{
		perror("opendir");
		return;
	}
	struct dirent *p;
	int flag = 0;
	int fd;
	long offset;
	while ((p=readdir(dir)) != NULL)
	{
		if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..") || (p->d_type == 4))
			continue;
		else if (!strcmp(p->d_name, sec))
		{	
			if (-1 == (fd=open(sec, O_RDWR)))
			{
				perror("open");
				return;
			}
			offset = lseek(fd, 0, SEEK_END);
			if (offset > 0)
			{	
				flag = 1;
				*total = offset;
				t->len = sprintf(t->buf, "%s %s %ld\n", fir,sec,offset);
				Sendn(sfd, (char *)t, 4 + t->len);
			}
			close(fd);
			break;
		}else{
			continue;
		}
	}
	if (0 == flag)
		Sendn(sfd, (char *)t, 4 + t->len);
}

void Findfile(tda *t, int sfd)
{
	tda temp;
	bzero(&temp, sizeof(tda));
	char *fir, *sec, *delim = " \n";
	memcpy(&temp, t, sizeof(tda));
	fir = strtok(t->buf, delim);
	sec = strtok(NULL, delim);
	DIR *dir;
	if (NULL == (dir=opendir(getcwd(NULL, 0))))
	{
		perror("opendir");
		return;
	}
	struct dirent *p;
	int flag = 0;
	while ((p=readdir(dir)) != NULL)
	{
		if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..") || (p->d_type == 4))
			continue;
		else if (!strcmp(p->d_name, sec))
		{	
			flag = 1;
			Sendn(sfd, (char *)&temp, 4 + temp.len);
			Sendfile(sfd, sec);
		}else{
			continue;
		}
	}
	if(0 == flag)
		printf("client no this file,please dectected your input!\n");
}

void Recvfilesize(int len, int sfd, long *size)
{	
	char bufs[1000] = {0};
	bzero(bufs, sizeof(bufs));
	Recvn(sfd, bufs, len);
	*size = atol(bufs);
	printf("size=%ld Byte\n", *size);
}

void Sendn(int fd, char *buf, int len)
{
	int total = 0;
	int pos;
	while (total < len)
	{
		pos = send(fd, buf + total, len - total, 0);
		total = total + pos;
	}
}

void Helpfun()
{
	puts("|----------------------------------------|");
	puts("|         ?         帮助信息             |");
	puts("|      cd or CD:    切换路径             |");
	puts("|      ls or LS:    显示当前目录详细信息 |");
	puts("|     pwd or PWD:   显示当前路径         |");
	puts("|    help or HELP   帮助信息             |");
	puts("|    gets or GETS:  下载服务器文件至本地 |");
	puts("|    quit or QUIT:  退出登录             |");
	puts("|    puts or PUTS:  本地文件上传至服务器 |");
	puts("|  remove or REMOVE:从服务器删除文件     |");
	puts("|----------------------------------------|");
}


