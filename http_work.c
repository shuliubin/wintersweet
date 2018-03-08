
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "http_work.h"

//http 消息头
#define HEAD "HTTP/1.0 200 OK\n\
Content-Type: %s\n\
Transfer-Encoding: chunked\n\
Connection: Keep-Alive\n\
Accept-Ranges:bytes\n\
Content-Length:%d\n\n"

//http 消息尾
#define TAIL "\n\n"

//输出文件时的头内容
#define SERVER_STRING "Server: pi@raspberrt\r\n"

extern char LOGBUF[1024];
void save_log(char *buf)
{
      FILE *fp = fopen("log.txt","a+");  
      fputs(buf,fp);  
      fclose(fp);  
}

const char *get_filetype(const char *filename) //根据扩展名返回文件类型描述
{
	////////////得到文件扩展名///////////////////
	char sExt[32];
	const char *p_start=filename;
	memset(sExt, 0, sizeof(sExt));
	while(*p_start)
	{
		if (*p_start == '.')
		{
			p_start++;
			strncpy(sExt, p_start, sizeof(sExt));
			break;
		}
		p_start++;
	}

	////////根据扩展名返回相应描述///////////////////

	if (strncmp(sExt, "bmp", 3) == 0)
		return "image/bmp";

	if (strncmp(sExt, "gif", 3) == 0)
		return "image/gif";

	if (strncmp(sExt, "ico", 3) == 0)
		return "image/x-icon";

	if (strncmp(sExt, "jpg", 3) == 0)
		return "image/jpeg";

	if (strncmp(sExt, "avi", 3) == 0)
		return "video/avi";

	if (strncmp(sExt, "css", 3) == 0)
		return "text/css";

	if (strncmp(sExt, "dll", 3) == 0)
		return "application/x-msdownload";

	if (strncmp(sExt, "exe", 3) == 0)
		return "application/x-msdownload";

	if (strncmp(sExt, "dtd", 3) == 0)
		return "text/xml";

	if (strncmp(sExt, "mp3", 3) == 0)
		return "audio/mp3";

	if (strncmp(sExt, "mpg", 3) == 0)
		return "video/mpg";

	if (strncmp(sExt, "png", 3) == 0)
		return "image/png";

	if (strncmp(sExt, "ppt", 3) == 0)
		return "application/vnd.ms-powerpoint";

	if (strncmp(sExt, "xls", 3) == 0)
		return "application/vnd.ms-excel";

	if (strncmp(sExt, "doc", 3) == 0)
		return "application/msword";

	if (strncmp(sExt, "mp4", 3) == 0)
		return "video/mpeg4";

	if (strncmp(sExt, "ppt", 3) == 0)
		return "application/x-ppt";

	if (strncmp(sExt, "wma", 3) == 0)
		return "audio/x-ms-wma";

	if (strncmp(sExt, "wmv", 3) == 0)
		return "video/x-ms-wmv";

	return "text/html";
}

int  get_file_content(const char *file_name, char **content) // 得到文件内容
{
	int  file_length = 0;
	FILE *fp = NULL;

	if (file_name == NULL)
	{
		return file_length;
	}

	fp = fopen(file_name, "rb");

	if (fp == NULL)
	{
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"file name: %s,%s,%d:open file failture %s \n",file_name, __FILE__, __LINE__,
				strerror(errno));
		save_log(LOGBUF);
		return file_length;
	}

	fseek(fp, 0, SEEK_END);
	file_length = ftell(fp);
	rewind(fp);

	*content = (char *) malloc(file_length);
	if (*content == NULL)
	{
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"%s,%d:malloc failture %s \n", __FILE__, __LINE__,
				strerror(errno));
		save_log(LOGBUF);
		return 0;
	}
	fread(*content, file_length, 1, fp);
	fclose(fp);

	return file_length;
}

//根据用户在GET中的请求，生成相应的回复内容
int make_http_content(const char *command, char **content)
{
	char *file_buf;
	int file_length;
	char headbuf[1024];
	if (command[0] == 0)
	{
		file_length = get_file_content("index.html", &file_buf);
	} else
	{
		file_length = get_file_content(command, &file_buf);
	}
	if (file_length == 0)
	{
		return 0;
	}

	memset(headbuf, 0, sizeof(headbuf));
	sprintf(headbuf, HEAD, get_filetype(command), file_length); //设置消息头

	int iheadlen = strlen(headbuf); //得到消息头长度
	int itaillen = strlen(TAIL); //得到消息尾长度
	int isumlen = iheadlen + file_length + itaillen; //得到消息总长度
	*content = (char *) malloc(isumlen); //根据消息总长度，动态分配内存
	if(*content==NULL)
	{
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"malloc failed %s\n", strerror(errno));
		save_log(LOGBUF);
	}
	char *tmp = *content;
	memcpy(tmp, headbuf, iheadlen); //安装消息头
	memcpy(&tmp[iheadlen], file_buf, file_length); //安装消息体
	memcpy(&tmp[iheadlen] + file_length, TAIL, itaillen); //安装消息尾
	//printf("headbuf:\n%s", headbuf);
	if (file_buf)
	{
		free(file_buf);
	}
	return isumlen; //返回消息总长度
}

//返回http的头部信息
void headers(int client, const char *filename) {  
    //先返回文件头部信息  
    char buf[1024];  
    (void) filename; /* could use filename to determine file type */  
  
    strcpy(buf, "HTTP/1.0 200 OK\r\n");  
    send(client, buf, strlen(buf), 0);  
    strcpy(buf, SERVER_STRING);  
    send(client, buf, strlen(buf), 0);  
    sprintf(buf, "Content-Type: image/jpeg\r\n");  
    send(client, buf, strlen(buf), 0);  
    strcpy(buf, "\r\n");  
    send(client, buf, strlen(buf), 0);  
} 

//直接返回文件内容  
void cat(int client, FILE *resource) {  
    //返回文件数据  
    char buf[1024];  
  
    fgets(buf, sizeof (buf), resource);  
    while (!feof(resource)) {  
        //循环发送文件  
        send(client, buf, strlen(buf), 0);  
        fgets(buf, sizeof (buf), resource);  
    }  
}

//得到http 请求中 GET后面的字符串
void get_http_command(char *http_msg, char *command)
{
	char *p_end = http_msg;
	char *p_start = http_msg;
	while (*p_start) //GET /
	{
		if (*p_start == '/')
		{
			break;
		}
		p_start++;
	}
	p_start++;
	p_end = strchr(http_msg, '\n');
	while (p_end != p_start)
	{
		if (*p_end == ' ')
		{
			break;
		}
		p_end--;
	}
	strncpy(command, p_start, p_end - p_start);
}

//开始http服务进程
void *http_thread(void *argc)
{
	printf("http_work begin \n");
	if(argc==NULL)
	{
		return NULL;
	}
	int st = *(int *) argc;
	free((int *)argc);
	char buf[1024];
	char path[512]; 
	memset(buf, 0, sizeof(buf));
	int rc = recv(st, buf, sizeof(buf), 0);
	if (rc <= 0)
	{
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"recv failed %s\n", strerror(errno));
		save_log(LOGBUF);
	} else
	{
		printf("recv:\n%s", buf);
		char command[1024];
		memset(command, 0, sizeof(command));
		get_http_command(buf, command); //得到http 请求中 GET后面的字符串
		printf("get:%s \n", command);
		///			
			if (strstr(command, "test.jpg") != NULL){
				printf("访问test.jpg!\n");
				FILE *resource = fopen(command, "r"); 
				if (resource == NULL)  
					return NULL;  ////////////////////////////////////////////////////////////////////////////////////////
				else {  
					//先返回头部信息  
					headers(st, command);
					cat(st, resource);  
				}  
				fclose(resource); 
			}
		///		
		char *content = NULL;
		int ilen = make_http_content(command, &content); //根据用户在GET中的请求，生成相应的回复内容
		if (ilen > 0)
		{
			send(st, content, ilen, 0); //将回复的内容发送给client端socket
			free(content);
		}
	}
	close(st); //关闭client端socket
	printf("http_work finished！\n");
	return NULL;
}

int socket_create(int port)
{
	int httpd = socket(AF_INET, SOCK_STREAM, 0);	/*建立 socket */
	if (httpd == -1)
	{
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"%s,%d:socker error %s\n", __FILE__, __LINE__, strerror(errno));
		save_log(LOGBUF);
		return 0;
	}

	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_port = htons(port); //指定一个端口号并将hosts字节型传化成Inet型字节型（大端或或者小端问题）
	sockaddr.sin_family = AF_INET;	//设置结构类型为TCP/IP
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);	//服务端是等待别人来连，不需要找谁的ip。这里写一个长量INADDR_ANY表示server上所有ip，这个一个server可能有多个ip地址，因为可能有多块网卡
	
	//将一本地地址与一套接口捆绑。
	if (bind(httpd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) == -1)
	{
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"%s,%d:bind error %s \n", __FILE__, __LINE__, strerror(errno));
		save_log(LOGBUF);
		return 0;
	}

	/*如果当前指定端口是 0，则动态随机分配一个端口*/  
    if (port == 0)  /* if dynamically allocating a port */  
    {  
        int namelen = sizeof(sockaddr);  
        if (getsockname(httpd, (struct sockaddr *)&sockaddr, &namelen) == -1) 
		{
			memset(LOGBUF,0,sizeof(LOGBUF));
			sprintf(LOGBUF,"%s,%d:getsockname error %s \n", __FILE__, __LINE__, strerror(errno));
			save_log(LOGBUF);
			return 0;
		}
        port = ntohs(sockaddr.sin_port);  
    } 	
	
	 // 	服务端开始监听
	if (listen(httpd, 100) == -1)
	{
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"%s,%d:listen failture %s\n", __FILE__, __LINE__,
				strerror(errno));
				save_log(LOGBUF);
		return 0;
	}
	printf("start server success,running on port %d!\n",port);
	return httpd;
}