#define _POSIX_C_SOURCE	200809L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#ifdef USE_LIBUNISTRING
#  include <unistr.h> /* per libunistring */
#endif
#include "rxb.h"
#include "utils.h"

#define MAX_REQUEST_SIZE (64 * 1024)

int main(int argc, char** argv){
    int err;
    struct addrinfo hints, *res, *ptr;
    char *host_remoto;
    char *servizio_remoto;
    int sd, i = 1;
    rxb_t rxb;

    /* Parameters check */
    if (argc < 3) {
            printf("Usage: client-td-connreuse <server> <port>\n");
            exit(EXIT_FAILURE);
    }

	/* Ignore SIGPIPE */
	signal(SIGPIPE, SIG_IGN);

    /* Address construction */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    /* Host resolution */
    host_remoto = argv[1];
    servizio_remoto = argv[2];
    if ((err = getaddrinfo(host_remoto, servizio_remoto, &hints, &res)) != 0) {
            fprintf(stderr, "Name resolution error: %s\n", gai_strerror(err));
            exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next){
        /*if socket creation fails, skip directly to the next iteration*/
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0){
                fprintf(stderr,"socket creation failed\n");
                continue;
        }
        /*if connect succeeds, exit the loop*/
        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0){
                printf("connect succeeded on attempt %d\n",i);
                break;
            }
        i++;
        close(sd);
    }

    /* Check the result returned by getaddrinfo */
    if (ptr == NULL) {
            fprintf(stderr, "Name resolution error: no matching address found\n");
            exit(EXIT_FAILURE);
    }

    /* Free the memory allocated by getaddrinfo() */
    freeaddrinfo(res);

    /* Initialize receive buffer */
    rxb_init(&rxb, MAX_REQUEST_SIZE);

    for (;;) {
        char email[4096], password[4096];

        /* Read data from the user */
        puts("Enter email, 'fine' to quit:");
        if (fgets(email, sizeof(email), stdin) == NULL) {
                perror("fgets");
                exit(EXIT_FAILURE);
        }

        /* Exit if the user types "fine" */
        if (strcmp(email, "fine\n") == 0) {
                break;
        }

        /* Send request to the Server */
        if (write_all(sd, email, strlen(email)) < 0) { 
                perror("write");
                exit(EXIT_FAILURE);
        }

        puts("Enter password:");
        if (fgets(password, sizeof(password), stdin) == NULL) {
                perror("fgets");
                exit(EXIT_FAILURE);
        }

        /* Send request to the Server */
        if (write_all(sd, password, strlen(password)) < 0) { 
                perror("write");
                exit(EXIT_FAILURE);
        }

        /* Read the server's response and print it to the screen */
        for (;;) {
            char response[MAX_REQUEST_SIZE];
            size_t response_len;

            /* Initialize the response buffer to zero and do not use the last
            * byte, so I am sure that the buffer content will always be
            * null-terminated. In this way, I can interpret it as a C string.
            * This is an operation that must be done before reading each new
            * response. */
            memset(response, 0, sizeof(response));
            response_len = sizeof(response)-1;

            /* Receive result */
            if (rxb_readline(&rxb, sd, response, &response_len) < 0) {
                /* If I am here, it means I have read an EOF. This means that
                    * the Server has closed the connection, so I deallocate
                    * rxb (optional) and exit. */
                rxb_destroy(&rxb);
                fprintf(stderr, "Connection closed by the server!\n");
                exit(EXIT_FAILURE);
            }

#ifdef USE_LIBUNISTRING
            /* Verify that the text is valid UTF-8 */
            if (u8_check((uint8_t *)response, response_len) != NULL) {
                /* Malfunctioning Server - sent response line
                * with invalid UTF-8 string */
                fprintf(stderr, "Response is not valid UTF-8!\n");
                close(sd);
                exit(EXIT_FAILURE);
            }
#endif

            /* Print content read from the Server */
            puts(response);

            /* Handle Access Denied --> Move to new request */
            if (strcmp(response, "--- Access denied ---") == 0) {
                break;
            }

            /* Move to new request once Server input is finished */
            if (strcmp(response, "--- END REQUEST ---") == 0) {
                break;
            }
        }
    }

    /* Close socket */
    close(sd);

    return 0;
}
