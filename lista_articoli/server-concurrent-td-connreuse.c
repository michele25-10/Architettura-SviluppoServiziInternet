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

/* SIGCHLD handler */
void handler(int signo)
{
        int status;

        (void)signo;

        /* Handle all terminated children */
        while (waitpid(-1, &status, WNOHANG) > 0)
                continue;
}

int autorizza(const char *email_revisore, const char *password)
{
        return 1;
}

int main(int argc, char **argv)
{
        struct addrinfo hints, *res;
        int err, sd, ns, pid, on;
        struct sigaction sa;

        /* Parameters check */
        if (argc != 2)
        {
                fprintf(stderr, "Usage: %s port\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        /* Ignore SIGPIPE */
        signal(SIGPIPE, SIG_IGN);

        /* Install SIGCHLD handler using the sigaction syscall, which is a
         * POSIX standard, instead of signal which has different semantics
         * depending on the operating system */
        sigemptyset(&sa.sa_mask);
        /* use SA_RESTART to avoid having to explicitly check if
         * accept was interrupted by a signal and in that case restart it
         * (see paragraph 21.5 of the book M. Kerrisk, "The Linux
         * Programming Interface") */
        sa.sa_flags = SA_RESTART;
        sa.sa_handler = handler;

        if (sigaction(SIGCHLD, &sa, NULL) == -1)
        {
                perror("sigaction");
                exit(EXIT_FAILURE);
        }

        memset(&hints, 0, sizeof(hints));
        /* Use AF_INET to force only IPv4, AF_INET6 to force only IPv6 */
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
        {
                fprintf(stderr, "Errore setup indirizzo bind: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }

        if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
                perror("Errore in socket");
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
                perror("Errore in bind");
                exit(EXIT_FAILURE);
        }

        freeaddrinfo(res);

        if (listen(sd, SOMAXCONN) < 0)
        {
                perror("listen");
                exit(EXIT_FAILURE);
        }

        /* Waiting clients... */
        for (;;)
        {
                printf("Server listening...\n");

                if ((ns = accept(sd, NULL, NULL)) < 0)
                {
                        perror("accept");
                        exit(EXIT_FAILURE);
                }

                /* Child generation */
                if ((pid = fork()) < 0)
                {
                        perror("fork");
                        exit(EXIT_FAILURE);
                }
                else if (pid == 0)
                {
                        int pid_n1, pid_n2, pipe_n1n2[2], status;
                        const char *end_request = "\n--- END REQUEST ---\n";
                        const char *access_denied = "\n--- Access denied ---\n";
                        rxb_t rxb;

                        /* child */
                        close(sd);

                        /* Disable SIGCHLD handler */
                        memset(&sa, 0, sizeof(sa));
                        sigemptyset(&sa.sa_mask);
                        sa.sa_handler = SIG_DFL;

                        if (sigaction(SIGCHLD, &sa, NULL) == -1)
                        {
                                perror("sigaction");
                                exit(EXIT_FAILURE);
                        }

                        /* Initialize receive buffer */
                        rxb_init(&rxb, MAX_REQUEST_SIZE);

                        /* Start request handling loop */
                        for (;;)
                        {
                                char email[4096], password[4096];
                                size_t email_len;
                                size_t password_len;

                                memset(email, 0, sizeof(email));
                                email_len = sizeof(email) - 1;

                                memset(password, 0, sizeof(password));
                                password_len = sizeof(password) - 1;

                                /* Read email */
                                if (rxb_readline(&rxb, ns, email, &email_len) < 0)
                                {
                                        /* If I am here, it means I have read an EOF. This means that
                                         * the Client has closed the connection, so I deallocate
                                         * rxb (optional) and exit. */
                                        rxb_destroy(&rxb);
                                        break;
                                }
#ifdef USE_LIBUNISTRING
                                /* Verify that the message is valid UTF-8 */
                                if (u8_check((uint8_t *)email, email_len) != NULL)
                                {
                                        /* Client malfunctioning - sent message
                                         * with invalid UTF-8 string */
                                        fprintf(stderr, "Request is not valid UTF-8!\n");
                                        close(ns);
                                        exit(EXIT_SUCCESS);
                                }
#endif
                                /* Read password */
                                if (rxb_readline(&rxb, ns, password, &password_len) < 0)
                                {
                                        /* If I am here, it means I have read an EOF. This means that
                                         * the Client has closed the connection, so I deallocate
                                         * rxb (optional) and exit. */
                                        rxb_destroy(&rxb);
                                        break;
                                }
#ifdef USE_LIBUNISTRING
                                /* Verify that the message is valid UTF-8 */
                                if (u8_check((uint8_t *)password, password_len) != NULL)
                                {
                                        /* Client malfunctioning - sent message
                                         * with invalid UTF-8 string */
                                        fprintf(stderr, "Request is not valid UTF-8!\n");
                                        close(ns);
                                        exit(EXIT_SUCCESS);
                                }
#endif

                                if (autorizza(email, password) != 1)
                                {
                                        if (write_all(ns, access_denied, strlen(access_denied)) < 0)
                                        {
                                                perror("write access denied");
                                                exit(EXIT_FAILURE);
                                        }
                                        continue;
                                }

                                /* Here goes the code to handle the authorized request */
                                if (pipe(pipe_n1n2) < 0)
                                {
                                        perror("pipe");
                                        exit(EXIT_FAILURE);
                                }

                                if ((pid_n1 = fork()) < 0)
                                {
                                        perror("fork");
                                        exit(EXIT_FAILURE);
                                }
                                else if (pid_n1 == 0)
                                {
                                        /* Grandchild n1 */
                                        close(pipe_n1n2[0]);
                                        close(1);
                                        if (dup(pipe_n1n2[1]) < 0)
                                        {
                                                perror("dup");
                                                exit(EXIT_FAILURE);
                                        }
                                        close(pipe_n1n2[1]);
                                        close(ns);

                                        execlp("grep", "grep", email, "test.txt", (char *)NULL);
                                        perror("exec grep");
                                        exit(EXIT_FAILURE);
                                }
                                if ((pid_n2 = fork()) < 0)
                                {
                                        perror("fork");
                                        exit(EXIT_FAILURE);
                                }
                                else if (pid_n2 == 0)
                                {
                                        /* Grandchild n2 */
                                        close(pipe_n1n2[1]);
                                        close(0);
                                        if (dup(pipe_n1n2[0]) < 0)
                                        {
                                                perror("dup");
                                                exit(EXIT_FAILURE);
                                        }
                                        close(pipe_n1n2[0]);
                                        close(1);
                                        if (dup(ns) < 0)
                                        {
                                                perror("dup");
                                                exit(EXIT_FAILURE);
                                        }
                                        close(ns);
                                        execlp("sort", "sort", "-r", "-n", (char *)NULL);
                                        perror("exec sort");
                                        exit(EXIT_FAILURE);
                                }
                                /* Child */
                                close(pipe_n1n2[0]);
                                close(pipe_n1n2[1]);
                                close(0);
                                if (dup(ns) < 0)
                                {
                                        perror("dup");
                                        exit(EXIT_FAILURE);
                                }

                                /* Wait for children termination */
                                waitpid(pid_n1, &status, 0);
                                waitpid(pid_n2, &status, 0);

                                /* Send a string to notify the end of the request */
                                if (write_all(ns, end_request, strlen(end_request)) < 0)
                                {
                                        perror("write");
                                        exit(EXIT_FAILURE);
                                }
                        }

                        /* Close the active socket */
                        close(ns);

                        /* Terminate the child */
                        exit(EXIT_SUCCESS);
                }
                /* Parent */
                close(ns);
        }

        /* Close the passive socket (just in case) */
        close(sd);
        return 0;
}