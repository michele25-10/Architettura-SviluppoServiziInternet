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
        continue;
}

int autorizza(char *username, char *password) { return 1; }

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
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
    {
        fprintf(stderr, "Errore risoluzione del nome %s", gai_strerror(err));
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
        fprintf(stdout, "Server in ascolto!\n");

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
            /** FIGLIO */
            char username[1024], password[1024], tipologia[1024];
            size_t len_username, len_password, len_tipologia;
            int pid1, pid2, status, p1p2[2];
            char *end_request = "\n--- END REQUEST ---\n";
            char *not_authorized = "Non autorizzato\n";
            rxb_t rxb;

            close(sd);

            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_DFL;
            if (sigaction(SIGCHLD, &sa, NULL) < 0)
            {
                perror("sigaction");
                exit(EXIT_FAILURE);
            }

            rxb_init(&rxb, MAX_REQUEST_SIZE);

            for (;;)
            {
                memset(username, 0, sizeof(username));
                len_username = sizeof(username) - 1;
                if (rxb_readline(&rxb, ns, username, &len_username) < 0)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)username, len_username) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_SUCCESS);
                }
#endif

                memset(password, 0, sizeof(password));
                len_password = sizeof(password) - 1;
                if (rxb_readline(&rxb, ns, password, &len_password) < 0)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)password, len_password) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_SUCCESS);
                }
#endif

                memset(tipologia, 0, sizeof(tipologia));
                len_tipologia = sizeof(tipologia) - 1;
                if (rxb_readline(&rxb, ns, tipologia, &len_tipologia) < 0)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)tipologia, len_tipologia) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_SUCCESS);
                }
#endif
                // inizio logica
                if (autorizza(username, password) != 1)
                {
                    if (write_all(ns, not_authorized, strlen(not_authorized)) < 0)
                    {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                    if (write_all(ns, end_request, strlen(end_request)) < 0)
                    {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                    continue;
                }

                char path[4096];
                snprintf(path, sizeof(path), "toys/%s.txt", tipologia);

                if (pipe(p1p2) < 0)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                if ((pid1 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid1 == 0)
                {
                    /** NIPOTE */
                    close(ns);
                    close(p1p2[0]);
                    close(1);
                    if (dup(p1p2[1]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p1p2[1]);

                    execlp("grep", "grep", "disponibile", path, (char *)NULL);
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }

                close(p1p2[1]);

                if ((pid2 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0)
                {
                    /** NIPOTE */
                    close(0);
                    if (dup(p1p2[0]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p1p2[0]);

                    close(1);
                    if (dup(ns) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(ns);

                    execlp("sort", "sort", "-n", (char *)NULL);
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }

                close(p1p2[0]);

                waitpid(pid1, &status, 0);
                waitpid(pid2, &status, 0);

                if (write_all(ns, end_request, strlen(end_request)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }

            rxb_destroy(&rxb);
            exit(EXIT_SUCCESS);
            close(ns);
        }

        close(ns);
    }

    close(sd);
    return 0;
}