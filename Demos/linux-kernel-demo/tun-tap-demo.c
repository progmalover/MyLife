/*
 * =====================================================================================
 *
 *       Filename:  tun-tap-demo.c
 *
 *    Description:  This is small demo to tell how tu use tun/tap device driver.
 *
 *        Version:  1.0
 *        Created:  02/01/2016 07:12:20 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  batmancn ,
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <linux/if_tun.h>

int tun_create(char *dev, int flag)
{
    int fd;
    // process fd into tun device
    return fd;
}

int main(int argc, const char *argv[])
{
    // make sure you have enough space.
    unsigned char buf[4096];

    // IFF_TUN means tun device,
    // IFF_NO_PI means do NOT transfer packet header into user space.
    tun = tun_create(tun_name, IFF_TUN | IFF_NO_PI);
    if (!tun) {
        // error quit
    }

    while(1){
        ret = read(tun, buf, sizeof(buf));  // use read system call
        if (!ret) {
            // error quit
        }

        // process buf

        ret = write(tun, buf, ret);
        if (!ret) {
            // error quit
        }
    }

    return 0;
}
