#include "ftp_server_func.h"

void Clientconnectblog(char *ip, int port, char *manual, int flag)
{
	char date[20] = {0};
	time_t t;
	time(&t);
	struct tm *pTm = gmtime(&t);
	sprintf(date, "%04d-%02d-%02d %02d:%02d:%02d", (1900+pTm->tm_year),\
		(1+pTm->tm_mon), pTm->tm_mday, (8+pTm->tm_hour), pTm->tm_min, pTm->tm_sec);
	if (0 == flag)
	{
		insertBlog(ip, port, date, manual, flag);
	}else{
		insertBlog(ip, port, date, manual, flag);
	}
}

int insertBlog(char *ip, int port, char *date, char *manual, int flag)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *server = "localhost";
	char *user = "root";
	char *password = "123";
	char *database = "ftp3";
	char query[200] = {0};
	if (0 == flag)
	{
		sprintf(query, "insert into blog(ip, port, date, manual) values('%s', '%d', '%s', 'request connect')", ip, port, date);
	}else if (1 == flag){
		sprintf(query, "insert into blog(ip, port, date, manual) values('%s', '%d', '%s', '%s')", ip, port, date, manual);
	}
	int t, r;
	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
	{
		printf("Error connecting to database:%s\n", mysql_error(conn));
		return -1;
	}
	t = mysql_query(conn, query);
	if (t)
	{
		printf("Error making query:%s\n", mysql_error(conn));
		return -1;
	}else{
		//printf("insert blog success\n");
		return 0;
	}
	mysql_close(conn);
	return 0;
}

int getbyName(char *username, char *salt, char *code)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *server = "localhost";
	char *user = "root";
	char *password = "123";
	char *database = "ftp3";
	char query[200] = {0};
	int t, r;
	sprintf(query, "select salt, code from login where username = '%s'", username);
	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
	{
		printf("Error connecting to database:%s\n", mysql_error(conn));
		return -1;
	}
	t = mysql_query(conn, query);
	if (t)
	{
		printf("Error making query:%s\n", mysql_error(conn));
		return -1;
	}else{
		res = mysql_use_result(conn);
		if (res)
		{
			row = mysql_fetch_row(res);
			if (row <= 0)
			{
				return -1;
			}
			if (row[0] && row[1])
			{
				strcpy(salt, row[0]);
				strcpy(code, row[1]);
				return 0;
			}else{
				return -1;
			}
		}else{
			return -1;
		}
		mysql_free_result(res);
	}
	mysql_close(conn);
	return 0;
}

int insertbyName(char *name, char *salt, char *code)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *server = "localhost";
	char *user = "root";
	char *password = "123";
	char *database = "ftp3";
	char query[200] = {0};
	sprintf(query, "insert into login(username, salt, code) values('%s', '%s', '%s')", name, salt, code);
	int t, r;
	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
	{
		printf("Error connecting to database:%s\n", mysql_error(conn));
		return -1;
	}
	t = mysql_query(conn, query);
	if (t)
	{
		printf("Error making query:%s\n", mysql_error(conn));
		return -1;
	}else{
		printf("insert success\n");
		return 0;
	}
	mysql_close(conn);
	return 0;
}

void getsalt(char salt[], int num)
{
	char *str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,./;\"'<>?";
	int i, len;
	char s[2] = {0};
	len = strlen(str);     //计算字符串长度
	srand((unsigned int)time((time_t *)NULL));
	for (i = 1; i <= num; i++)
	{
		sprintf(s, "%c", str[(rand() % len)]);
		strcat(salt, s);
	}
}

int Loginconfirm(ptda t, int new_sfd)
{
	bzero(t, sizeof(tda));
	struct spwd *sp;
	char salt[12] = {0};
	char code[100] = {0};
	char name[20] = {0};    //注册用户名用
	char pwd[20] = {0};     //注册用户时接收用户密码
	int ret;
	Recvn(new_sfd, (char *)&t->len, 4);
	Recvn(new_sfd, t->buf, t->len);
	ret = getbyName(t->buf, salt, code);
	if (-1 == ret)
	{
		t->len = -331;      //发一个结束标示符
		Sendn(new_sfd, (char *)&t->len, 4);
		printf("错误的用户名!\n");
		Recvn(new_sfd, (char *)&t->len, 4);
		Recvn(new_sfd, pwd, t->len);
		Recvn(new_sfd, (char *)&t->len, 4);
		Recvn(new_sfd, name, t->len);
		//printf("----%s----\n",name);
		getsalt(salt, 11);
		strcpy(code, crypt(pwd, salt));
		ret = insertbyName(name, salt, code);
		if (-1 == ret)
		{
			printf("insert\n");
			return -1;
		}else{
			printf("添加成功!\n");
			return 0;
		}
	}else{
		t->len = 331;
		Sendn(new_sfd, (char *)&t->len, 4);
		t->len = strlen(salt);
		Sendn(new_sfd, (char *)&t->len, 4);     //(char*)&strlen(salt);
		Sendn(new_sfd, (char *)salt, t->len);
		bzero(t, sizeof(tda));
		Recvn(new_sfd, (char *)&t->len, 4);
		Recvn(new_sfd, t->buf, t->len);
		if (!strcmp(t->buf, code))
		{
			t->len = 230;
			Sendn(new_sfd, (char*)&t->len, 4);
			printf("验证通过!\n");
			return 0;
		}else{
			t->len = -230;
			Sendn(new_sfd, (char*)&t->len, 4);
			printf("验证失败!\n");
			return -1;
		}
	}
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

int Sendfilesize(int new_sfd, int fd)
{
	struct stat buf;
	int ret = fstat(fd, &buf);
	if (-1 == ret)
	{
		perror("fstat");
		return -1;    //备注
	}
	tda t;
	printf("size=%ld Byte\n", buf.st_size);
	char bufs[20];
	bzero(bufs, sizeof(bufs));
	sprintf(bufs, "%ld", buf.st_size);
	t.len = strlen(bufs);
	strcpy(t.buf, bufs);
	Sendn(new_sfd, (char *)&t, 4 + t.len);
	return buf.st_size;
}

void Sendfile(int new_sfd, char *sec, long offset)
{   
	tda t;
	memset(&t, 0, sizeof(t));
	int fd; 
	long size;
	char *pmmap, *pcur;
	if (-1 == (fd = open(sec, O_RDONLY)))
	{ 
		perror("open");
		return;
	}
	size = Sendfilesize(new_sfd, fd);
	if (size > 100 * 1024 * 1024)
	{
		pmmap = (char *)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
		if ((char *)-1 == pmmap)
		{
			perror("mmap failed");
			return;
		}
		pcur = pmmap;
		pcur += offset;
		while (pmmap + size - pcur > 1000)
		{
			bzero(&t, sizeof(tda));
			memcpy(t.buf, pcur, 1000);
			t.len = 1000;
			Sendn(new_sfd, (char *)&t, 4 + t.len);
			pcur += 1000;
		}
		bzero(&t, sizeof(tda));
		t.len = (pmmap + size - pcur);
		memcpy(t.buf, pcur, 4 + t.len);
		Sendn(new_sfd, (char *)&t, 4 + t.len);
		if (-1 == munmap(pmmap, size))
		{
			perror("munmap");
			return;
		}
	}else{
		lseek(fd, offset, SEEK_SET);
		while (memset(&t, 0, sizeof(t)), (t.len = read(fd, t.buf, sizeof(t.buf))) > 0)
		{   
			Sendn(new_sfd, (char *)&t, 4 + t.len);
		}
	}
	t.len = -1;
	Sendn(new_sfd, (char *)&t.len, 4);    //发送结束符给客户端
	//close(new_sfd);
}

void Recvfd(int fdr, int *fd, struct sockaddr_in *client)
{
	struct msghdr msg;
	bzero(&msg, sizeof(msg));
	struct iovec iov[2];
	char buf[20] = "wang dao-"; 
	char buf1[20] = "belief dream";
	iov[0].iov_base = buf;
	iov[0].iov_len = 9;
	iov[1].iov_base = buf1;
	iov[1].iov_len = 12;
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;
	struct cmsghdr *cmsg;
	int len = CMSG_LEN(sizeof(int));
	cmsg = (struct cmsghdr *)calloc(1, len);
	cmsg->cmsg_len = len;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	msg.msg_control = cmsg;
	msg.msg_controllen = len;
	int ret = recvmsg(fdr, &msg, 0);
	if (-1 == ret)
	{
		perror("recvmsg");
		return;
	}
	*fd = *(int *)CMSG_DATA(cmsg);
}

void handle_request(int fdr)
{
	tda t;
	int new_sfd;
	short flag = 1;
	char *fir, *sec, *thr, *delim = " \n";
	int len;
	struct sockaddr_in client;
	char buf[50] = {0};
	long offset = 0;
	int ret;
	while (1)
	{
		Recvfd(fdr, &new_sfd, &client);
		printf("new_sfd=%d\n", new_sfd);
		ret = Loginconfirm(&t, new_sfd);
		if (-1 == ret){
			//printf("---%d---\n",ret);
			write(fdr, &flag, sizeof(short));
			//printf("---%d---\n",ret);
			//return;
		}
		while (1)
		{	
			bzero(&t, sizeof(tda));
			bzero(buf, sizeof(buf));
			Recvn(new_sfd, (char *)&len, 4);
			Recvn(new_sfd, buf, len);
			buf[strlen(buf) - 1] = 0;
			Clientconnectblog(inet_ntoa(client.sin_addr), ntohs(client.sin_port), buf, flag);
			if (!strncmp(buf, "ls", 2) || !strncmp(buf, "LS", 2))
			{
				Lsfun(getcwd(NULL, 0), new_sfd);
			}else if (!strncmp(buf, "pwd", 3) || !strncmp(buf, "PWD", 3)){
				Pwdfun(new_sfd);
			}else if (!strncmp(buf, "cd", 2) || !strncmp(buf, "CD", 2)){
				fir = strtok(buf, delim);
				sec = strtok(NULL, delim);
				Cdfun(sec, new_sfd);
			}else if (!strncmp(buf, "gets", 4) || !strncmp(buf, "GETS", 4)){
				fir = strtok(buf, delim);
				sec = strtok(NULL, delim);
				if (NULL == sec)
				{
					printf("error input,please detect your input!\n");
					continue;
				}
				thr = strtok(NULL, delim);
				if (NULL != thr)
					offset = atol(thr);
				Getsfun(sec, new_sfd, offset);
				offset = 0;
			}else if (!strncmp(buf, "puts", 4) || !strncmp(buf, "PUTS", 4)){
				fir = strtok(buf, delim);
				sec = strtok(NULL, delim);
				Putsfun(sec, new_sfd);
			}else if (!strncmp(buf, "quit", 4) || !strncmp(buf, "QUIT", 4)){
				printf("exit connect!\n");
				close(new_sfd);
				break;
			}else if (!strncmp(buf, "remove", 6) || !strncmp(buf, "REMOVE", 6)){
				fir = strtok(buf, delim);
				sec = strtok(NULL, delim);
				if (NULL == sec)
				{
					printf("error input,please detect your input!\n");
					continue;
				}
				Removefun(sec, new_sfd);
			}else{
				continue;
			}
		}
		write(fdr, &flag, sizeof(short));
	}
}

void Recvfilesize(int new_sfd, long *size)
{
	int len;
	char bufs[1000];
	bzero(&bufs, sizeof(bufs));
	Recvn(new_sfd, (char *)&len, 4);
	Recvn(new_sfd, bufs, len);
	*size = atol(bufs);
	printf("size=%ld Byte\n", *size);
}

void Putsfun(char name[], int new_sfd)
{
	int fd, len;
	time_t first, second;
	long size, total=0;
	double i=5, j;
	char buf[1000] = {0};
	if (-1 == (fd = open(name, O_RDWR|O_CREAT, 0666)))
	{
		perror("open");
		return;
	}
	Recvfilesize(new_sfd, &size);
	first = time((time_t *)NULL);
	printf("3.0%%......\n");
	while (1)
	{
		Recvn(new_sfd, (char *)&len, 4);
		total += len;
		bzero(buf, sizeof(buf));
		if (len > 0)
		{
			Recvn(new_sfd, buf, len);
			write(fd, buf, len);
		}else if (-1 == len){
			printf("100%%......\n");
			printf("puts success!\n");
			break;
		}
		second = time((time_t *)NULL);
		j = difftime(second, first);
		if (j > i)
		{
			printf("%3.0f%%......\n", (float)total / size * 100);
			i += 5;
		}
	}
	close(fd);
}

void Removefun(char name[], int new_sfd)
{	
	int len;
	if (!(len = remove(name)))
		Sendn(new_sfd, (char *)&len, 4);
	else
	{
		len = -1;
		Sendn(new_sfd, (char *)&len, 4);
	}
}

void Getsfun(char name[], int new_sfd, long offset)
{
	DIR *dir;
	if (NULL == (dir = opendir(getcwd(NULL, 0))))   
	{   
		perror("opendir");
		return;
	}   
	struct dirent *p; 
	int flag = 0;
	while ((p = readdir(dir)) != NULL)
	{   
		if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..") || (p->d_type == 4))
			continue;
		else if (!strcmp(p->d_name, name))
		{
			flag = 1;
			Sendfile(new_sfd, name, offset);
		}else{
			continue;
		}
	}
	if (0 == flag)
		Sendn(new_sfd, (char *)&flag, 4);
	closedir(dir);
}


void Pwdfun(int new_sfd)
{
	tda t;
	bzero(&t, sizeof(tda));
	t.len = strlen(getcwd(NULL, 0));
	strcpy(t.buf, getcwd(NULL, 0));
	Sendn(new_sfd, (char *)&t, 4 + t.len);
}

void Cdfun(char *sec, int new_sfd)
{
	tda t;
	bzero(&t, sizeof(tda));
	int len = -1;
	if (-1 == chdir(sec))
	{
		perror("chdir");
		Sendn(new_sfd, (char *)&len, 4);
		return;
	}else{
		Pwdfun(new_sfd);
	}
}

void Makechild(pcda pc, int count)
{
	int i, fds[2];
	pid_t pid;
	for (i = 0; i < count; i++)
	{
		socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
		pid = fork();
		if (0 == pid)
		{	
			close(fds[1]);
			handle_request(fds[0]);
		}
		pc[i].pid = pid;
		pc[i].fdw = fds[1];
		pc[i].busy = 0;
		close(fds[0]);
	}
}

void Sendfd(int fdw, int fd, struct sockaddr_in client)
{
	struct msghdr msg;
	bzero(&msg, sizeof(msg));
	struct iovec iov[2];
	char buf[20] = "wang dao-"; 
	char buf1[20] = "belief dream";
	iov[0].iov_base = buf;
	iov[0].iov_len = 9;
	iov[1].iov_base = buf1;
	iov[1].iov_len = 12;
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;
	struct cmsghdr *cmsg;
	int len = CMSG_LEN(sizeof(int));
	cmsg =(struct cmsghdr *)calloc(1, len);
	cmsg->cmsg_len = len;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	*(int*)CMSG_DATA(cmsg) = fd;
	msg.msg_control = cmsg;
	msg.msg_controllen = len;
	int ret=sendmsg(fdw, &msg, 0);
	if (-1 == ret)
	{
		perror("sendmsg");
		return;
	}
}

void Eventinit(struct epoll_event *ev, int fd)
{
	bzero(ev, sizeof(struct epoll_event));
	ev->events = EPOLLIN;
	ev->events = EPOLLIN;
	ev->data.fd = fd;
}

int Tcpinit(char *ip, char *port)
{
	int ret, sfd;
	if (-1 == (sfd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		perror("socket");
		return -1;
	}
	int val = 1;
	ret = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
	if (-1 == ret)
	{
		printf("setsockopt");
	}
	struct sockaddr_in sev; 	
	bzero(&sev, sizeof(struct sockaddr_in));
	sev.sin_family = AF_INET;
	sev.sin_port = htons(atoi(port));
	sev.sin_addr.s_addr = inet_addr(ip);
	ret = bind(sfd, (struct sockaddr *)&sev, sizeof(struct sockaddr_in));
	if (-1 == ret)
	{
		perror("bind");
		return -1;
	}
	return sfd;
}

void Getrwx(int *_rwx, char *str)                                                      
{
	int i;
	for (i = 0; i < 9; i++)
		if (*_rwx & 1<<i)
			if (i == 0 || i == 3 || i == 6)
				str[9-i] = 'x';
			else if (i == 1 || i == 4 || i == 7)
				str[9-i] = 'w';
			else if (i == 2 || i == 5 || i == 8)
				str[9-i] = 'r';

}

void Getfiletype(int *type, char *str)
{
	if(S_ISDIR(*type))
		str[0] = 'd';
	else if(S_ISLNK(*type))
		str[0] = 'l';
	else if(S_ISREG(*type))
		str[0] = '-';
	else if(S_ISCHR(*type))
		str[0] = 'c';
	else if(S_ISBLK(*type))
		str[0] = 'b';
	else if(S_ISFIFO(*type))
		str[0] = 'f';
	else if(S_ISSOCK(*type))
		str[0] = 's';
}

void Lsfun(char path[], int new_sfd)
{
	struct stat buf;
	DIR *dir;
	if ((dir=opendir(path)) == NULL)
	{
		perror("opendir");
		return;
	}
	char str[10];
	tda t;
	struct dirent *p;
	int ret;
	int len = -1;
	struct tm *pT;
	while ((p = readdir(dir)) != NULL)
	{
		bzero(&t, sizeof(tda));
		memset(&buf, 0, sizeof(buf));
		bzero(str, sizeof(str));
		strcpy(str, "----------");
		ret = stat(p->d_name, &buf);
		if (ret == -1)
		{
			perror("stat");
			return;
		}
		Getfiletype((int *)&buf.st_mode, str);  //备注：强转
		Getrwx((int *)&buf.st_mode, str);       //同上
		pT = gmtime(&buf.st_mtime);
		t.len = sprintf(t.buf, "%s %2d %4s %4s %10ld %4d-%02d-%02d %02d:%02d:%02d %-30s\n",\
				str, buf.st_nlink, getpwuid(buf.st_uid)->pw_name, getgrgid(buf.st_gid)->gr_name, buf.st_size,\
				(1900+pT->tm_year), (1+pT->tm_mon), pT->tm_mday, (8+pT->tm_hour), pT->tm_min, pT->tm_sec, p->d_name);
		Sendn(new_sfd, (char *)&t, 4 + t.len);
	}
	Sendn(new_sfd, (char *)&len, 4);
	closedir(dir);
} 


