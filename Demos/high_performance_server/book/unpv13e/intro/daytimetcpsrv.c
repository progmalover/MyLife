// Basic server loop:
// listenfd = create socket
// fill serveraddr
// bind addr to listenfd
// for :
//     connfd = accept from listenfd
//     read from connfd
//
// Why connfd and listenfd?
//     As mutilduplex. Linux use file like to manage, detail have to read book.

#include	"unp.h"
#include	<time.h>

int
main(int argc, char **argv)
{
    // 1. listenfd and connfd
    // 2. servaddr used to connect
    // 3. receive buffer
	int					listenfd, connfd;
	struct sockaddr_in	servaddr;
	char				buff[MAXLINE];
	time_t				ticks;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(13);	/* daytime server */

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	for ( ; ; ) {
		connfd = Accept(listenfd, (SA *) NULL, NULL);

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        Write(connfd, buff, strlen(buff));

		Close(connfd);
	}
}
