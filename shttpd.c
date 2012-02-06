#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <errno.h>

int get_line(int, char *, int);
void bad_request(int);
void cannot_execute(int);

int main() {
    int sock0;
    struct sockaddr_in addr;
    struct sockaddr_in client;
    socklen_t len;
    int yes = 1;

    char buf[2048];
    char inbuf[2048];

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

    /* 応答メッセージ */
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), 
	"HTTP/1.1 100 Continue\r\n"
	"Content-Length: 7\r\n"
	"Content-Type: text/html\r\n"
	"\r\n"
	"HELLO\r\n");
    /* TCPクライアントからの接続要求を受け付ける */
    while (1) {
    
	len = sizeof(client);
	sock_lis = accept(sock0, (struct sockaddr *)&client, &len);
	if (sock_lis < 0) {
	    perror("ERROR: cannot accept.\r\n");
	    break;
	}

	
        /* 文字送信 */
	memset(inbuf, 0, sizeof(inbuf));
	recv(sock_lis, inbuf, sizeof(inbuf), 0); // TODO use get_line
	printf("%s", inbuf);

	int ok;
	ok = send(sock_lis, buf, (int)strlen(buf), MSG_DONTWAIT);
	printf("sent bytes = %d¥r¥n", ok);

	if (strstr(inbuf, "Connection: close") != NULL) {
	    close(sock_lis);
	    printf("connection is closed.¥r¥n");
	    break;
	} else {
	    printf("continue the session¥r¥n");
	}
    }
    close(sock_lis);
    close(sock0);

    return 0;
}

int get_line(int sock, char *buf, int size) {

    int i = 0;
    char c = '¥0';
    int n;

    while ((i < size -1) && (c != '¥n')) {
	n = recv(sock, &c, 1, 0);

	if (n > 0) {
	    if (c == '¥r') {
		n = recv(sock, &c, 1, MSG_PEEK);
		if ((n > 0) && (c == '¥n')) {
		    recv(sock, &c, 1, 0);
		} else {
		    c = '¥n';
	        }
	    } else {
		buf[i] = c;
		i++;
	    }
	} else {
	    c = '¥n';
	}
    }
    buf[i] = '¥0';
    return(i);

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

