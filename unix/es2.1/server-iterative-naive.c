#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{
        int sd, err, on;
        struct addrinfo hints, *res;

        /* Parameter check */
        if (argc != 2)
        {
                fprintf(stderr, "Usage: %s port\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        /* Ignore SIGPIPE */
        signal(SIGPIPE, SIG_IGN);

        /* Prepare getaddrinfo */
        memset(&hints, 0, sizeof(hints));
        /* Use AF_INET to force only IPv4, AF_INET6 to force only IPv6 */
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        /* Use getaddrinfo to prepare the data structures to use with socket and bind */
        if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
        {
                fprintf(stderr, "Error setting up bind address: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }

        /* Create socket */
        if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
                perror("Error creating socket");
                exit(EXIT_FAILURE);
        }

        /* Disable waiting time before socket creation */
        on = 1;
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        {
                perror("setsockopt");
                exit(EXIT_FAILURE);
        }

        /* Socket listening in the desired port */
        if (bind(sd, res->ai_addr, res->ai_addrlen) < 0)
        {
                perror("Error binding socket");
                exit(EXIT_FAILURE);
        }

        /* At this point, I can free the memory allocated by getaddrinfo */
        freeaddrinfo(res);

        /* Transform into passive listening socket */
        if (listen(sd, SOMAXCONN) < 0)
        {
                perror("listen");
                exit(EXIT_FAILURE);
        }

        /* Request handling loop - iterative server with transient connection */
        for (;;)
        {
                char request[4096], response[256];
                int ns, nread, length_of_string;

                /* Wait for connection requests */
                ns = accept(sd, NULL, NULL);
                if (ns < 0)
                {
                        /* I have not installed SIGCHLD handler with SA_RESTART,
                         * so I have to explicitly check and handle the
                         * EINTR case */
                        if (errno == EINTR)
                                continue;
                        perror("accept");
                        exit(EXIT_FAILURE);
                }

                /* Initialize the request buffer to zero and do not use the last
                 * byte, so I am sure that the content of the buffer will be
                 * always null-terminated. In this way, I can interpret it
                 * as a C string and pass it directly to the function
                 * strlen. This is an operation that must be performed before
                 * each new request. */
                memset(request, 0, sizeof(request));

                /* Read request from Client */
                if ((nread = read(ns, request, sizeof(request) - 1)) < 0)
                {
                        perror("read");
                        close(ns);
                        continue;
                }

                /* String length computation */
                length_of_string = strlen(request);

                /* Prepare the response buffer */
                snprintf(response, sizeof(response), "%d\n", length_of_string);

                /* Send the response */
                if (write(ns, response, strlen(response)) < 0)
                {
                        perror("write");
                        close(ns);
                        continue;
                }

                /* Close active socket */
                close(ns);
        }

        /* Close passive socket (just in case) */
        close(sd);

        return 0;
}
