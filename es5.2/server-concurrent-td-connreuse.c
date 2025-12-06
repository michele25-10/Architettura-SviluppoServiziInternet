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

    while (waitpid(-1, &status, WNOHANG) == -1)
    {
        continue;
    }
}

int autorizza(const char *email_revisore, const char *password)
{
    return 1;
}

int main(int argc, char **argv)
{
    int sd, ns, pid, err, on;
    struct addrinfo hints, *res;
    struct sigaction sa;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: server <porta>\n");
        exit(0);
    }

    signal(SIGPIPE, SIG_IGN);

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) < 0)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) < 0)
    {
        fprintf(stderr, "Error Risolution Naming!\n");
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
        perror("bind");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server in ascolto!\n");

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
            /** FIGLIO */
            char *end_request = "\n--- END REQUEST ---\n";
            char email[256], password[1024], rivista[1024];
            int pid1, pid2, status;
            int p1p2[2], p2p0[2];
            size_t len_email, len_password, len_rivista;
            rxb_t rxb;

            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_IGN;

            if (sigaction(SIGCHLD, &sa, NULL) < 0)
            {
                perror("sigaction");
                exit(EXIT_FAILURE);
            }

            rxb_init(&rxb, MAX_REQUEST_SIZE);

            for (;;)
            {
                memset(email, 0, sizeof(email));
                len_email = sizeof(email) - 1;
                if (rxb_readline(&rxb, ns, email, &len_email) < 0)
                {
                    rxb_destroy(&rxb);
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)email, len_email) != NULL)
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
                    rxb_destroy(&rxb);
                    perror("rxb_readline");
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

                memset(rivista, 0, sizeof(rivista));
                len_rivista = sizeof(rivista) - 1;
                if (rxb_readline(&rxb, ns, rivista, &len_rivista) < 0)
                {
                    rxb_destroy(&rxb);
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)rivista, len_rivista) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_SUCCESS);
                }
#endif

                if (autorizza(email, password) == 0)
                {
                    close(ns);
                    exit(EXIT_FAILURE);
                }

                // CREO LA PIPE
                if (pipe(p1p2) < 0)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                if ((pid1 = fork()) < 0)
                {
                    perror("pid1");
                    exit(EXIT_FAILURE);
                }
                else if (pid1 == 0)
                {
                    int pf[2];
                    int pidf;
                    if (pipe(pf) < 0)
                    {
                        perror("pp");
                        exit(EXIT_FAILURE);
                    }
                    close(ns);
                    close(p1p2[0]);

                    if ((pidf = fork()) < 0)
                    {
                        perror("pidf");
                        exit(EXIT_FAILURE);
                    }
                    else if (pidf == 0)
                    {
                        /** FIGLIO */
                        close(pf[0]);
                        close(p1p2[1]);
                        close(1);

                        if (dup(pf[1]) < 0)
                        {
                            perror("dup");
                            exit(EXIT_FAILURE);
                        }
                        close(pf[1]);

                        execlp("grep", "grep", email, "data.txt", (char *)NULL);
                        perror("grep pidf");
                        exit(EXIT_FAILURE);
                    }

                    /**PADRE */
                    close(pf[1]);
                    close(0);
                    if (dup(pf[0]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(pf[0]);

                    close(1);
                    if (dup(p1p2[1]) < 1)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p1p2[1]);

                    execlp("grep", "grep", rivista, (char *)NULL);
                    perror("grep p1p2");
                    exit(EXIT_FAILURE);
                }

                close(p1p2[1]);

                if (pipe(p2p0) < 0)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                if ((pid2 = fork()) < 0)
                {
                    perror("pid2");
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0)
                {
                    close(ns);
                    close(p2p0[0]);
                    close(0);

                    if (dup(p1p2[0]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p1p2[0]);

                    close(1);
                    if (dup(p2p0[1]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p2p0[1]);

                    execlp("sort", "sort", "-n", "-r", (char *)NULL);
                    perror("sort");
                    exit(EXIT_FAILURE);
                }

                close(p1p2[0]);
                close(p2p0[1]);

                wait(&status);
                wait(&status);

                char line[MAX_REQUEST_SIZE];
                if (read_all(p2p0[0], line, sizeof(line)) < 0)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }

                close(p2p0[0]);

                if (write_all(ns, line, strlen(line)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }

                int n = 0;
                for (int idx = 0; idx < strlen(line); idx++)
                {
                    if (line[idx] == '\n')
                        n++;
                }

                sprintf(line, "Numero di riviste: %d\n", n);

                if (write_all(ns, line, strlen(line)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }

                if (write_all(ns, end_request, strlen(end_request)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }

            close(ns);
            exit(EXIT_SUCCESS);
        }

        /** PADRE */
        close(ns);
    }

    close(sd);
    return 0;
}