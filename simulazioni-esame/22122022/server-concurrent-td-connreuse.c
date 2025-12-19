#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#ifdef USE_LIBUNISTRING
#include <unistr.h> /* per libunistring */
#endif
#include "utils.h"
#include "rxb.h"

#define MAX_REQUEST_SIZE (64 * 1024)

void handler(int signo)
{
    int status;
    (void)signo;
    while (waitpid(-1, &status, WNOHANG) > 0)
    {
        continue;
    }
}

int main(int argc, char **argv)
{
    int sd, ns, err, on, pid;
    struct addrinfo hints, *res;
    struct sigaction sa;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: server <port>\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    sa.sa_restart = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // get addrinfo

    close(sd);
    return 0;
}