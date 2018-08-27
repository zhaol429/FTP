#include "ftp_client_func.h"
#include "ftp_client_func.cpp"

int main(int argc, char **argv)
{
	if (3 != argc)
	{
		printf("error args");
		return -1;
	}
	//int idx;
	int sfd = Tcpinit(argv[1], argv[2]);
	int fd;
	int len;
	char buf[1000] = {0}, temp[1000];
	long size, total = 0;
	double i = 5, j;
	time_t first, second;
	char *fir, *sec, *delim = " \n";
	tda t;
	int ret;
	ret = Loginrequest(&t, sfd);
	if (-1 == ret){
		close(sfd);
		return 0;
	}
	//Helpfun();
	while (1)
	{	//int val=0;
		bzero(&t, sizeof(tda));
		bzero(buf, sizeof(buf));
		t.len = read(0, t.buf, sizeof(t.buf));
		if (!strncmp(t.buf, "ls", 2) || !strncmp(t.buf, "LS", 2))
		{
			Sendn(sfd, (char *)&t, 4 + t.len);
			while (1)
			{
				Recvn(sfd, (char *)&len, 4);
				bzero(buf, sizeof(buf));
				if (len > 0)
				{  
					Recvn(sfd, buf, len);
					write(STDOUT_FILENO, buf, len);
				}else{
					break;
				}
			}
		}else if (!strncmp(t.buf, "pwd", 3) || !strncmp(t.buf, "PWD", 3)){
			Sendn(sfd, (char *)&t, 4 + t.len);
			Recvn(sfd, (char *)&len, 4);
			Recvn(sfd, buf, len);
			write(STDOUT_FILENO, buf, len);
			printf("\n");
		}else if (!strncmp(t.buf, "cd", 2) || !strncmp(t.buf, "CD", 2)){
			Sendn(sfd, (char *)&t, 4 + t.len);
			bzero(buf, sizeof(buf));
			Recvn(sfd, (char *)&len, 4);
			if (len <= 0)
			{
				printf("open failed,error dirrent!\n");
			}else if (len > 0){
				Recvn(sfd, buf, len);
				write(STDOUT_FILENO, buf, len);
				printf("\n");
			}
		}else if (!strncmp(t.buf, "gets", 4) || !strncmp(t.buf, "GETS", 4)){
			bzero(temp, sizeof(temp));
			memcpy(temp, t.buf, sizeof(t.buf));
#if 0
			idx = 6;
			while (t.buf[idx] != '\0')
			{
				if (t.buf[idx] == ' ' || t.buf[idx] == '\n'){
					++idx;
					continue;
				}
				else
					val = 1;
			}
			if (val == 0){
				printf("无效的输入!");
				continue;
			}
#endif
			fir = strtok(temp, delim);
			sec = strtok(NULL, delim);
			//puts(fir);
			//puts(sec);
			Getscontinue(&t, sfd, &total);
			//Sendn(sfd,(char*)&t,4+t.len);
			Recvn(sfd, (char *)&len, 4);
			if (0 == len)
			{
				printf("sever no find this file!\n");
			}else if (len > 0){
				printf("续传点:%ld\n", total);
				Recvfilesize(len, sfd, &size);
				if (total > 0)
				{
					fd = open(sec, O_RDWR|O_APPEND, 0666);
					if (-1 == fd)
					{
						perror("open");
						return -1;
					}
				}else{
					fd = open(sec, O_RDWR|O_CREAT, 0666);
					if (-1 == fd)
					{
						perror("open");
						return -1;
					}
				}
				first = time((time_t *)NULL);
				while (1)
				{
					Recvn(sfd, (char *)&len, 4);
					total += len;
					bzero(buf, sizeof(buf));
					if (len > 0)
					{
						Recvn(sfd, buf, len);
						write(fd, buf, len);
					}else if (-1 == len){
						printf("100%%......\n");
						printf("gets success!\n");
						total = 0;
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
		}else if (!strncmp(t.buf, "puts", 4) || !strncmp(t.buf, "PUTS", 4)){	
			Findfile(&t, sfd);
		}else if (!strncmp(t.buf, "quit", 4) || !strncmp(t.buf, "QUIT", 4)){
			printf("exit success!\n");
			close(sfd);
			exit(0);
		}else if (!strncmp(t.buf, "remove", 6) || !strncmp(t.buf, "REMOVE", 6)){
			Sendn(sfd, (char *)&t, 4 + t.len);
			Recvn(sfd, (char *)&len, 4);
			if (-1 == len)
			{
				printf("remove failed,please dectected your input!\n");
			}else{
				printf("remove success!\n");
			}
		}else if (!strncmp(t.buf, "?", 1)){
			Helpfun();
		}else if (!strncmp(t.buf, "help", 4) || !strncmp(t.buf, "HELP", 4)){
			Helpfun();
		}else{
			puts("无效的输入!");
		}
		fflush(stdin);
	}
	close(sfd);
	return 0;
}
