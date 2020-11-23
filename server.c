#include "unp.h"
#include <ctype.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#define pic_MAXLEN 10000005
#define SERVER_PORT 8080
char TCP_head[] =  	"HTTP/1.1 200 OK\r\n";
char TCP_head_send_jpg[] =  "HTTP/1.1 200 OK\r\n"
					"Content-Type: image/jpeg\r\n";
char HTML1[] =		"\r\n"
					"<!DOCTYPE html>\r\n"
					"<html><head><title>408410061_Web_server</title>\r\n"
					"</head>\r\n"
					"<body>\r\n";
char select_img[] = "<form action=\"\" method=\"post\" enctype=\"multipart/form-data\">\r\n"
					"Please specify a image<br>\r\n"
					"<input type=\"file\" name=\"img1\">\r\n"
					"<button type=\"submit\">Submit</button>\r\n"
					"</form>\r\n";
char text[] =	  	"<img src=\"/img1.jpeg\" alt=\"No img\">\r\n";
char HTML2[] =  	"</body></html>\r\n";
char buff[pic_MAXLEN + 5];
char picture[pic_MAXLEN + 5];
void strcat_pack_selc(char *pack)
{
	pack[0] = '\0';
	strcat(pack, TCP_head);
	strcat(pack, HTML1);
	strcat(pack, select_img);
	strcat(pack, text);
	strcat(pack, HTML2);
}
// void strcat_pack_show_img(char *pack)
// {
// 	pack[0] = '\0';
// 	strcat(pack, TCP_head);
// 	strcat(pack, HTML1);
// 	strcat(pack, text);
// 	strcat(pack, HTML2);
// }
char *get_picture(char *package_head)
{
	char *c = package_head;
	c = strstr(c, "\r\n\r\n");
	c = c + 5;
	c = strstr(c, "\r\n\r\n");
	c = c + 4;
	return c;
}
int get_len(char *pack)
{
	char *line = strstr(pack, "Content-Length: ");
	char n[10];
	int idx = 0;
	while(*(++line) != '\n')
	{
		if(isdigit(*line)) n[idx++] = *line;
	}
	n[idx] = 0;
	return atoi(n);
}
void picture_pack(char *pack)
{
	pack[0] = '\0';
	strcat(pack, TCP_head_send_jpg);
	strcat(pack, (char *)picture);
}
void show_pack_msg(char *pack)
{
	if(strncmp(buff, "GET ", 4) == 0 && strstr(buff, "GET /img1.jpeg")) printf("Browser request Image\n");
	else if(strncmp(buff, "GET ", 4) == 0 && strstr(buff, "Accept: text/html")) printf("Browser request Index\n");
	else if(strncmp(buff, "POST ", 5) == 0 && strstr(buff, "multipart/form-data")) printf("Browser send a file\n");
	else printf("Browser send a packet\n");
}
int listenfd;

int connfd;

void sigintHandler(int sig_num) 
{ 
	Close(listenfd);
	Close(connfd);

	printf("Close Server\n");

	exit(0);
} 

int main(int argc, char **argv)
{
	signal(SIGINT, sigintHandler);
	struct sockaddr_in servaddr;
	char pack[MAXLINE];

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERVER_PORT); 

	Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);
	printf("Listen Successful\n");

	signal(SIGCHLD, SIG_IGN);

	while(1)
	{
		connfd = Accept(listenfd, (SA *)NULL, NULL);
		printf("Accept Packet\n");

		pid_t pid = Fork();
		if (!pid)
		{
			Read(connfd, buff, pic_MAXLEN);
			printf("%s\n", buff);
			pack[0] = '\0';
			
			show_pack_msg(buff);
			
			if(strncmp(buff, "GET ", 4) == 0 && strstr(buff, "GET /img1.jpeg")) 
			{
				
				FILE *image = Fopen("img1.jpeg", "rb");
				int image_fd = fileno(image);

				struct stat statbuf;
				fstat(image_fd, &statbuf);

				strcat(pack, TCP_head_send_jpg);
				sprintf(buff, "Content-Length: %d\r\n", (int) statbuf.st_size);
				strcat(pack, buff);
				strcat(pack, "\r\n");
				Write(connfd, pack, strlen(pack));
				sendfile(connfd, image_fd, 0, statbuf.st_size);
				pack[0] = '\0';
			}
			else if(strncmp(buff, "GET ", 4) == 0 && strstr(buff, "Accept: text/html")) 
			{
				strcat_pack_selc(pack);
				Write(connfd, pack, strlen(pack));
			}
			else if(strncmp(buff, "POST ", 5) == 0 && strstr(buff, "multipart/form-data"))
			{
				char *start = get_picture(buff);
				int len = get_len(buff);
				FILE *fp = Fopen("img1.jpeg", "w");
				
				printf("\n%d\n", len);
				fwrite(start, (sizeof(char) * len), 1, fp);				
				strcat_pack_selc(pack);
				Write(connfd, pack, strlen(pack));
			}
			else
			{
				strcat_pack_selc(pack);
				Write(connfd, pack, strlen(pack));
			}
			
			pack[0] = '\0';
			buff[0] = '\0';
			Close(connfd);
			exit(0);
		}
		else
		{
			Close(connfd);
		}
	}
}
