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
    struct addrinfo hints, *res;
    struct sigaction sa;
    int sd, err, ns, pid, on;

    if (argc != 2)
    {
        printf("Usage: %s, port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_TYPE;
    hints.ai_flags = AI_PASSIVE;

    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) < 0)
    {
        fprintf(stderr, "Errore bind address\n");
        exit(EXIT_FAILURE);
    }

    if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    on = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(sd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if (listen(sd, SOMAXCONN) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        if ((ns = accept(sd, NULL, NULL)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        if ((pid = fork()) < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            const char *end_request = "\n--- END REQUEST ---\n";
            char search_string[4096];
            size_t len_search_string;
            char filename[4096];
            size_t len_filename;
            int status;
            int pid2;
            rxb_t rxb;

            close(sd);

            memset(&sa, 0, sizeof(sa));
            sa.sa_handler = SIG_DFL;

            if (sigaction(SIGCHLD, &sa, NULL) < 0)
            {
                perror("Errore SIGCHLD");
                exit(EXIT_FAILURE);
            }

            rxb_init(&rxb);

            memset(search_string, 0, sizeof(search_string));
            len_search_string = sizeof(search_string) - 1;

            if (rxb_readline(&rxb, sd, search_string, &len_search_string) < 0)
            {
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }

            memset(filename, 0, sizeof(filename));
            len_filename = sizeof(filename) - 1;

            if (rxb_readline(&rxb, sd, filename, &len_filename) < 0)
            {
                rxb_destroy(&rxb);
                exit_failure(EXIT_FAILURE);
            }

            if ((pid2 = fork()) < 0)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if (pid2 == 0)
            {
                close(stdout);

                if (dup(ns) < 0)
                {
                    perror("dup");
                    exit(EXIT_FAILURE);
                }

                close(ns);

                execlp("rgrep", "rgrep", search_string, filename, (char *)NULL);
                perror("rgrep");
                exit(EXIT_FAILURE);
            }

            wait(&status);

            if (write_all(ns, end_request, strlen(end_request)) < 0)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }

            close(ns);
            exit(EXIT_SUCCESS);
        }

        /* Padre */
        close(ns);
    }

    close(sd);
    return 0;
}