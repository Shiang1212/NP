#include <stdio.h>  
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  
#include <signal.h>
#include <sys/types.h>
#include <sys/sendfile.h>  
#include <sys/socket.h>  
#include <sys/stat.h>
#include <netinet/in.h>  
#include <arpa/inet.h>  
  
char buff[20480005]={0};
char TCP_head[] =  	"HTTP/1.1 200 OK\r\n";
char TCP_head_send_jpg[] =  "HTTP/1.1 200 OK\r\n"
					"Content-Type: image/jpeg\r\n";
char HTML1[] =		"\r\n"
					"<!DOCTYPE html>\r\n"
					"<html><head><title>server</title>\r\n"
					"</head>\r\n"
					"<body>\r\n";
char select_img[] =	"<form action=\"\" method=\"post\" enctype=\"multipart/form-data\">\r\n"
					"Please submit a image<br>\r\n"
					"<input type=\"file\" name=\"img\">\r\n"
					"<button type=\"submit\">Submit</button>\r\n"
					"</form>\r\n";
char text[] =		"<img src=\"/img.jpeg\" alt=\"No img\">\r\n";
char HTML2[] =		"</body></html>\r\n";
void strcat_pack_selc(char *pack) 
{
	pack[0] = '\0';
	strcat(pack, TCP_head);
	strcat(pack, HTML1);
	strcat(pack, select_img);
	strcat(pack, text);
	strcat(pack, HTML2);
}
char *get_picture(char *package_head) 
{
	char *c = package_head;
	c = strstr(c, "\r\n\r\n");
	c = c + 5;
	c = strstr(c, "\r\n\r\n");
	c = c + 4;
	return c;
}
char picture[20480005]={0};
int get_len(char *pack) 
{
	char *line = strstr(pack, "Content-Length: ");
	char n[10];
	int idx = 0;
	while(*(++line) != '\n') {
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
	if(strncmp(buff, "GET ", 4) == 0 && strstr(buff, "GET /img.jpeg")) printf("Browser request Image\n");
	else if(strncmp(buff, "GET ", 4) == 0 && strstr(buff, "Accept: text/html")) printf("Browser request Index\n");
	else if(strncmp(buff, "POST ", 5) == 0 && strstr(buff, "multipart/form-data")) printf("Browser send a file\n");
	else printf("Browser send a packet\n");
}
int fd; 
int sock0;  

int main()  
{  

  struct sockaddr_in addr;  
  char pack[4096];

  
  sock0 = socket(AF_INET, SOCK_STREAM, 0);  
  
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;  
  addr.sin_port = htons(12345);  
  addr.sin_addr.s_addr = htonl(INADDR_ANY);  
  
  bind(sock0, (struct sockaddr*)&addr, sizeof(addr));  
  printf("\t[Info] binding...\n");  
  
  listen(sock0, 1024);  
  printf("\t[Info] listening...\n");  
  signal(SIGCHLD, SIG_IGN);
  bzero(&pack, sizeof(pack));
  bzero(&buff, sizeof(buff));
  while(1)
  {
    fd = accept(sock0, (struct sockaddr*)NULL, NULL);
    pid_t pid = fork();
    if(pid == -1)
    {
      printf("fork error\n");
      return 87;
    }
    if(!pid)
    {
      read(fd, buff, 20480000);
      printf("%s\n",buff);

      show_pack_msg(buff);

      if(strncmp(buff, "GET ", 4) == 0 && strstr(buff, "GET /img.jpeg"))
      {
        FILE *image = fopen("img.jpeg", "rb");
        int image_fd = fileno(image);

        struct stat statbuf;
        fstat(image_fd, &statbuf);

        strcat(pack, TCP_head_send_jpg);
        sprintf(buff, "Content-Length: %d\r\n", (int) statbuf.st_size);
        strcat(pack, buff);
				strcat(pack, "\r\n");
				
				write(fd, pack, strlen(pack));

        sendfile(fd, image_fd, 0, statbuf.st_size);
        bzero(&pack, sizeof(pack));
        fclose(image);
      }
      else if(strncmp(buff, "GET ", 4) == 0 && strstr(buff, "Accept: text/html"))
      {
        strcat_pack_selc(pack);
				write(fd, pack, strlen(pack));
      }
      else if(strncmp(buff, "POST ", 5) == 0 && strstr(buff, "multipart/form-data"))
      {
				char *start = get_picture(buff);
				int len = get_len(buff);
				FILE *fp = fopen("img.jpeg", "w");
				printf("\n%d\n", len);
				fwrite(start, (sizeof(char) * len), 1, fp);				
				strcat_pack_selc(pack);
				write(fd, pack, strlen(pack));
        fclose(fp);
			} 
      else 
      {
				strcat_pack_selc(pack);
				write(fd, pack, strlen(pack));
			}
      bzero(&pack, sizeof(pack)*4);
      bzero(&buff, sizeof(buff)*4);
      pack[0] = '\0';
			buff[0] = '\0';
      close(fd);
      return 0;
    } 
    else {
			if(close(fd) == -1) {
				printf("close error\n");
				return 0;
			}
		}
  }
}  