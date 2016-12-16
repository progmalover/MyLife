// Reference:
// 1. http://pubs.opengroup.org/onlinepubs/009695399/functions/poll.html

// Poll is to allow app to listen lots of socket at same time.
//
// This is used in TCP long time connection. As in daytimesrv demo, client
// use short connection, which means do not holds fd. But if client code
// like "str_echo" of "str_echo09.c", the connfd will keep same in server,
// so server could not close it.
//
// As above said, connfd array need keep and listen at then same time, so
// loop and listen is not good idea, that's why linux soppurt Poll. Before
// use Poll, server is like "str_echo" of "str_echo09.c", which is as bellow.
// And also server have to use multi-thread to do these code, most of time is
// useless loop.
// for (loop) :
//     if (n = Read(from connfd)):
//         Process data
//         Writen(to fd)
//
// After use Poll, linux kernel do loop and check fd work, once there are data
// comes, kernel will write lable bit of this fd. So app has no need to use
// multi-thread to loop itself.
//
// So Poll is not only used for socket, also used for any event like timer.

/* include fig01 */
#include	"unp.h"
#include	<limits.h>		/* for OPEN_MAX */

int
main(int argc, char **argv)
{
	int					i, maxi, listenfd, connfd, sockfd;
	int					nready;
	ssize_t				n;
	char				buf[MAXLINE];
	socklen_t			clilen;

    // 1. define "struct pollfd" array, OPEN_MAX may be 4094.
	struct pollfd		client[OPEN_MAX];

	struct sockaddr_in	cliaddr, servaddr;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

    // 2. format client, [0] is listenfd struct, contains fd and poll mechanism
    // refer to <1>.
    //
    // maxi is the count of fd we need to listen, so init is 0 connfd and 1
    // listenfd. With connfd increase, maxi will be increase.
	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	for (i = 1; i < OPEN_MAX; i++)
		client[i].fd = -1;		/* -1 indicates available entry */
	maxi = 0;					/* max index into client[] array */
/* end fig01 */

/* include fig02 */
	for ( ; ; ) {
		nready = Poll(client, maxi+1, INFTIM);

		if (client[0].revents & POLLRDNORM) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
#ifdef	NOTDEF
			printf("new client: %s\n", Sock_ntop((SA *) &cliaddr, clilen));
#endif

			for (i = 1; i < OPEN_MAX; i++)
				if (client[i].fd < 0) {
					client[i].fd = connfd;	/* save descriptor */
					break;
				}
			if (i == OPEN_MAX)
				err_quit("too many clients");

			client[i].events = POLLRDNORM;
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}

		for (i = 1; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i].fd) < 0)
				continue;
			if (client[i].revents & (POLLRDNORM | POLLERR)) {
				if ( (n = read(sockfd, buf, MAXLINE)) < 0) {
					if (errno == ECONNRESET) {
							/*4connection reset by client */
#ifdef	NOTDEF
						printf("client[%d] aborted connection\n", i);
#endif
						Close(sockfd);
						client[i].fd = -1;
					} else
						err_sys("read error");
				} else if (n == 0) {
						/*4connection closed by client */
#ifdef	NOTDEF
					printf("client[%d] closed connection\n", i);
#endif
					Close(sockfd);
					client[i].fd = -1;
				} else
					Writen(sockfd, buf, n);

				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
}
/* end fig02 */
