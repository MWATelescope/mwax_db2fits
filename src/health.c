/**
 * @file health.c
 * @author Greg Sleap
 * @date 4 Jul 2018
 * @brief This is the code that provides health information to M&C
 *
 */
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <unistd.h>

#include "health.h"
#include "global.h"

#define PORT 7123
#define SERVER "127.0.0.1"

void* health_thread_fn(void *args)
{
    health_thread_args_t *health_args = (health_thread_args_t*)args;

    multilog(health_args->log, LOG_INFO, "Health: Thread started.\n");

    // Initialise UDP socket          
    int sock;

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        multilog(health_args->log, LOG_ERR, "Health: Could not open a socket to health IP: call to socket() failed.\n");
        exit(EXIT_FAILURE);
    }

    // initialise the destination address struct
    struct sockaddr_in si_other;
    int slen=sizeof(si_other);
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        multilog(health_args->log, LOG_ERR, "Health: Could not open a socket to health IP: call to inet_aton() failed.\n");
        exit(EXIT_FAILURE);        
    }

    // Test message    
    int quit = 0;

    while (!quit)
    {                
        // Check quit status
        quit = get_quit();    
        multilog(health_args->log, LOG_DEBUG, "%d bytes.\n", sizeof(health_thread_args_t));    

        //send the message
        if (sendto(sock, health_args, sizeof(health_thread_args_t) , 0, (struct sockaddr *) &si_other, slen) == -1)
        {
            multilog(health_args->log, LOG_ERR, "Health: Could not open a socket to health IP: call to sendto() failed.\n");
            exit(EXIT_FAILURE);        
        }

        // Wait for 1 second
        sleep(HEALTH_SLEEP_SECONDS);                
    }

    multilog(health_args->log, LOG_DEBUG, "Health: Closing socket.\n");
    if (sock)
    {
        close(sock);
    }

    multilog(health_args->log, LOG_INFO, "Health: Thread finished.\n");
}

