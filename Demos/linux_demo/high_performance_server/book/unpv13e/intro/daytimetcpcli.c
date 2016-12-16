// Basic tcp based client demo.

#include	"unp.h"

int
main(int argc, char **argv)
{
    // 1. define sockfd as socket file, n as read buffer return size.
    // 2. recvline as read buffer.
    // 3. servaddr as socket param.
	int					sockfd, n;
	char				recvline[MAXLINE + 1];
	struct sockaddr_in	servaddr;

	if (argc != 2)
		err_quit("usage: a.out <IPaddress>");

    // First create TCP socekt, which family is AF_INET, socket type is SOCK_STREAM.
    // sockfd is socket file handler.
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("socket error");

    // set servaddr to connect, 1. set sin_family as AF_INET; 2. set sin_port
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(13);	/* daytime server */

    // set IPAddress in argv[1] into &servaddr.sin_addr
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		err_quit("inet_pton error for %s", argv[1]);

    // connect, as linux treat all as file, so after connect, use sockfd to
    // read/write stream.
    //
    // Detail of connect function is
    // int
    // connect(int sockfd UNUSED, void *addr, int addr_len), addr type use
    // void is because there will be sockaddr_in and sockaddr6_in.
	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
		err_sys("connect error");

    // use sockfd as file handler.
    // recvline point to buffer, MAXLINE is max size of this buffer.
	while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        // null terminate is neccessary, as there is no recvline buffer length
        // in fputs function.
		recvline[n] = 0;	/* null terminate */
		if (fputs(recvline, stdout) == EOF)
			err_sys("fputs error");
	}
	if (n < 0)
		err_sys("read error");

	exit(0);
}
