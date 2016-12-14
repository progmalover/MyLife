// Reference:
// 1. http://www.cnblogs.com/shanks/archive/2009/08/23/1552552.html
// 2. <linux tcpip协议栈分析>

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
    // First alloc values:
    // listenfd and connfd.
    // servaddr used to connect.
    // receive buffer.
	int					listenfd, connfd;
	struct sockaddr_in	servaddr;
	char				buff[MAXLINE];
	time_t				ticks;

    // Create listenfd in file system, its type is AF_INET, and socket type is
    // SOCK_STREAM, flag is 0.
    // After create, this socket is TCP socket with TCP FMT with initialize
    // state.
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    // Second create server socket and set its value, such as:
    // sin_family, AF_INET
    // sin_addr.s_addr, dst_ip of packet, INADDR_ANY means accept all IP of client.
    // sin_port, listened port of this server.
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(13);	/* daytime server */

    // Third bind listenfd into this server socket, this is bind file-like
    // handler with socketaddr_in. After bind, TCP FMT of listenfd is still
    // init state.
	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

    // Fourth call Listen API, this is to set TCP FMT of listenfd into LISTEN
    // state, refer to <1>. LISTENQ is length of listen buffer in kernel.
	Listen(listenfd, LISTENQ);

    // loop until app close this server, by CTRL+C or other signal.
	for ( ; ; ) {
        // call Accept API of this file-like listenfd, which set listenfd tobe
        // acceptable??? Read <2> to get more.
        // Server will block in Accept until there is a connfd. This is done
        // by linux kernel.
        // Do not ask stupid question such as "why not check connfd valid?",
        // it's valid all the time.
		connfd = Accept(listenfd, (SA *) NULL, NULL);

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));

        // there is a connection, its file handler is connfd, app just write
        // data to connfd, just like write data to file.
        Write(connfd, buff, strlen(buff));

        // This is time server, so app close connfd after write. Notice, listefd
        // will not close, it's manage by kernel? No, but listenfd is used in
        // hole life of server, so no need to close it. If you really want to
        // close it, use Close is OK.
		Close(connfd);
	}
}
