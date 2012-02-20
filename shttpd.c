#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "filereader.h"

#define ISspace(x) isspace((int)(x))

struct req{
    char *firstline[1024];
    char *headers[1024];
    char *bodies[2048];
};

int get_line(int, char *, int);
void bad_request(int);
void cannot_execute(int);
char *check_servmap(char *);
struct req read_req(int, int);


/* return the servletname from requested url*/
char *check_servmap(char *req) {
    // TODO have to implement
    return "HelloServlet";
}

int main() {
    int sock0;
    struct sockaddr_in addr;
    struct sockaddr_in client;
    socklen_t len;
    int yes = 1;

    struct req httpreq;
    char inbuf[1024];
    char inheadbuf[2048];
    char headerbuf[1024];
    char method[255];
    char url[255];
    char path[512];

    char resheader[2048];
    char rescontent[2024];
    
    int sock_lis;
    int result;

    /* ソケット生成(TCP/IP, stream) */
    sock0 = socket(AF_INET, SOCK_STREAM, 0);
    if (sock0 < 0) {
	perror("ERROR: cannot create socket.\r\n");
	return 1;
    }

    /* ソケット設定 */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_len = sizeof(addr);

    setsockopt(sock0, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
    
    result = bind(sock0, (struct sockaddr *) &addr, sizeof(addr));
    if (result != 0) {
	perror("ERROR: cannot bind.\r\n");
	return 1;
    }

    /* クライアントからの接続待ち */
    if (listen(sock0, 5) != 0) {
	perror("ERROR: cannot listen.");
	return 1;
    }

    /* TCPクライアントからの接続要求を受け付ける */
    while (1) {
    
	len = sizeof(client);
	sock_lis = accept(sock0, (struct sockaddr *)&client, &len);
	if (sock_lis < 0) {
	    perror("ERROR: cannot accept.\r\n");
	    break;
	}

	
        /* 文字受信 */
	
	get_line(sock_lis, inbuf, sizeof(inbuf));
	size_t i = 0, j = 0;
	while (!ISspace(inbuf[j]) && (i < sizeof(method)) -1) {
	    method[i] = inbuf[j];
	    i++; j++;
	}
	method[i] = '\0';
	// TODO have to judge the method
	
	// Get the indicated URL
	i = 0;
	while (ISspace(inbuf[j]) && (j < sizeof(inbuf)))
	    j++;
	while (!ISspace(inbuf[j]) && (i < sizeof(url) - 1) && (j < sizeof(inbuf))) {
	    url[i] = inbuf[j];
	    i++; j++;
	}
	url[i] = '\0';

	char *servletName;
	servletName = check_servmap(url);
	if (servletName == NULL) {
	    // TODO implement
	    return -1;
	}


	// recv(sock_lis, inbuf, sizeof(inbuf), 0); // TODO use get_line
	printf("%s", inbuf);


	if (strcasecmp(servletName, "HelloServlet")) {
	    
	    OpenAndRead("hello.txt","r", rescontent);
	    if (*rescontent == '\0') {
		printf("no file.\n");
		return -1;
	    }
	}

	
	/* 応答メッセージ */
	sprintf(resheader,
	    "HTTP/1.1 100 Continue\r\n"
	    "Content-Length: 100\r\n"
	    "Content-Type: text/html\r\n"
	    "\r\n");

        /* 文字送信 */
	int hlen, blen;
	hlen = send(sock_lis, headerbuf, (int)strlen(headerbuf), MSG_DONTWAIT);
	blen = send(sock_lis, rescontent, (int)strlen(rescontent), MSG_DONTWAIT);
	printf("sent bytes = %d\r\n", hlen + blen);

	if (strstr(inbuf, "Connection: close") != NULL) {
	    close(sock_lis);
	    printf("connection is closed.\r\n");
	    break;
	} else {
	    printf("continue the session\r\n");
	}
    }
    close(sock_lis);
    close(sock0);

    return 0;
}

int get_line(int sock, char *buf, int size) {

    int i = 0;
    char c = '\0';
    int n;

    while ((i < size -1) && (c != '\0')) {
	n = recv(sock, &c, 1, 0);

	if (n > 0) {
	    if (c == '\r') {
		n = recv(sock, &c, 1, MSG_PEEK);
		if ((n > 0) && (c == '\n')) {
		    recv(sock, &c, 1, 0);
		} else {
		    c = '\n';
	        }
	    } else {
		buf[i] = c;
		i++;
	    }
	} else {
	    c = '\n';
	}
    }
    buf[i] = '\0';
    return(i);

}

struct req read_req(int sock, int size) {

    struct req httpreq;
    char buf[1024];
    int firstl = 0;
    
    firstl = get_line(sock, buf, size);
    if (firstl == 0) return httpreq;
    httpreq.firstline[0] = buf;

    int headers = 0;
    char headbuf[1024];
    int h;
    while (1) {
	
	char headbuf[1024];
	get_line(sock, headbuf, size);
	if (strcmp(headbuf, "\r") || strcmp(headbuf, "\r\n")) break;
	httpreq.headers[h] = headbuf;
	h++;
    }

    int b;
    char bodybuf[1024];
    while (1) {
	get_line(sock, bodybuf, size);
	if (strcmp(bodybuf, "\0")) break;
	httpreq.bodies[b] = bodybuf;
	b++;
    }

    return httpreq;

}


void bad_request(int client) {
    char buf[1024];

    sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "<P>Your browser sent a bad request, ");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "such as a POST without a Content-Length.\r\n");
    send(client, buf, sizeof(buf), 0);
}

void cannot_execute(int client) {
    char buf[1024];

    sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
    send(client, buf, strlen(buf), 0);
}

