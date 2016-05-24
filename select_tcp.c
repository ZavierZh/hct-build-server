#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/select.h>
#include<unistd.h>

#define strcase(a,b) strncmp(a,b,sizeof(a)-1)
#define BUFF_SIZE  			1024
#define PATH_MIN_LEN 		20
#define ECHO_OUT 0


#define STAT_START 1
#define STAT_WAIT  2
#define STAT_RUN   3

#define STAT_CLOSE 8
#define STAT_failed 9


char *path="./data";


typedef struct {
	char type;
	char data[BUFF_SIZE];	
}data_t;

int open_fifo(char *path){
	int fw;
 	if(mkfifo(path,0664) == -1){
		if(errno == EEXIST){
			fw = open(path,O_RDONLY);	
		}else {
			perror("error:mkfifo");
			return errno;
		}
	}else{
		fw = open(path,O_RDONLY);
	}
	return fw;
}

int main(int argc, const char *argv[]){
	int tcp_socket;
	unsigned char buff[BUFF_SIZE] = {0};
	unsigned char cur_path[BUFF_SIZE] = {0};
	struct sockaddr_in myaddr,peeraddr;
	int peer_len;
	int recv_len;
	
	if((tcp_socket = socket(PF_INET,SOCK_STREAM,0)) == -1){
		perror("socket");
		exit(1);
	}
	memset(&myaddr,0,sizeof(myaddr));
	
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(9101);
	myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	memset(&peeraddr,0,sizeof(peeraddr));
	peer_len = sizeof(peeraddr);
	if(-1 ==  connect(tcp_socket,(struct sockaddr *)&myaddr,peer_len)){
		perror("server");
		exit(1);
	}
	
	int fw;
	int len;

 	if(mkfifo(path,0664) == -1){
		if(errno == EEXIST){
			fw = open(path,O_RDONLY);	
		}else {
			perror("error:mkfifo");
			exit(1);
		}
	}else{
		fw = open(path,O_RDONLY);
	}
	puts("start-run");
	fputs("----*",stderr);
	if((len = read(fw,buff,BUFF_SIZE)) <= 0){
		fprintf(stderr,"error:read fialed(len.%d)\n",len);
		goto error;
	}
	fputs("******",stderr);
	buff[len] = '\0';
	if((len < 9) || (strncmp("name:",buff,5) != 0)){
		fputs("error:name is short or null",stderr);
		goto error;
	}
	send(tcp_socket,buff,len,0);
	//len = recv(tcp_socket,buff,BUFF_SIZE,0);
	len = recv(tcp_socket,buff,BUFF_SIZE,0);
	buff[len] = '\0';
	if(strcmp("success",buff) != 0){
		fputs("error:server refuse,set name failed",stderr);
		goto error;
	}
	
/***********************************/
	int max = tcp_socket>fw ? tcp_socket : fw;
	fd_set readfds,tmpfds;
//------
	FD_ZERO(&readfds);
	FD_SET(fw,&readfds);
	FD_SET(tcp_socket,&readfds);

//-----------------------------------------
	int n;
	int stat;
	while(1){
		tmpfds = readfds;
		n=select(max+1,&tmpfds,NULL,NULL,NULL);
		fprintf(stderr,"-------%d----------\n",n);
		if(FD_ISSET(tcp_socket,&tmpfds)){
			len = recv(tcp_socket,buff,BUFF_SIZE,0);
//			printf("recv len:%d\n",len);
			if(len <= 0){
				fprintf(stderr,"error:recv failed(len.%d)\n",len);
				goto error;
			}else if(len > 0){
				buff[len]='\0';
				if(strcase("start_build:",buff) == 0){
					//int web_fd;
					//web_fd=0;
					char *tmpbuff = buff + sizeof("start_build:")-1;
					while ((*tmpbuff != ';' )&&(*tmpbuff != '\0' )){
						tmpbuff++;
					}
					//tmpbuff++;
					//*(tmpbuff++) = 0;
					//web_fd = atoi(buff+12);
					
					if(strlen(tmpbuff) < PATH_MIN_LEN){
						*tmpbuff = '\0';
						strcpy(cur_path,buff+12);
						len = sprintf(buff,"re_start_build:%s;failed;path length is less than %d\n",path,PATH_MIN_LEN);
						send(tcp_socket,buff,len,0);
					}else{
						tmpbuff++;
						printf("start_build:%s",tmpbuff);

						if((len = read(fw,buff,BUFF_SIZE)) <= 0){
							fprintf(stderr,"error:read fialed(len.%d)\n",len);
							send(tcp_socket,"close\n",7,0);
							goto error;
						}
						buff[len]='\0';
						fputs(buff,stderr);
						strcpy(path,tmpbuff);

						cur_path[len-12] = '\0';
						fputs(cur_path,stderr);
					}
				}
				//puts(buff);	
			}
		}else if(FD_ISSET(fw,&tmpfds)){
			//memset(buff,0,BUFF_SIZE);
			if((len = read(fw,buff,BUFF_SIZE)) <= 0){
				fprintf(stderr,"error:read fialed(len.%d)\n",len);
				send(tcp_socket,"close\n",7,0);
				goto error;
			}
			buff[len]='\0';
			if(strcmp("exit\n",buff) == 0){
				fputs("Bye.",stderr);
				goto error;
			}
			fprintf(stderr,"recv_fifo:%s",buff);
			continue;
			if((len = send(tcp_socket,buff,len,0)) <= 0){
				fprintf(stderr,"error:send fialed(len.%d)\n",len);
				goto error;
			}
		}else if (n < 0){
			perror("select");
			goto error;
		}else {
			fprintf(stderr,"select:other(%d)(fw.%d,tcp.%d)\n",n,fw,tcp_socket);
		}
	}

	return 0;
error:

	close(tcp_socket);
	close(fw);
	exit(1);
}
