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
#include <time.h>
#include "health.h"
#include "version.h"

/**
 * 
 *  @brief This is the main health thread function to send health out_udp_data for this process via UDP.
 *  @param[in] args Pointer to the arguments that main() passes to this function.
 *  @returns void. 
 */
void *health_thread_fn(void *args)
{
    // Get time of launch
    time_t start_time = time(NULL);

    health_thread_data_s *health_args = (health_thread_data_s *)args;

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
    int slen = sizeof(struct sockaddr_in);
    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;

    si_other.sin_port = htons(health_args->health_udp_port);

    if (inet_aton(health_args->health_udp_ip, &si_other.sin_addr) == 0)
    {
        multilog(health_args->log, LOG_ERR, "Health: Could not open a socket to health IP: call to inet_aton() failed.\n");
        exit(EXIT_FAILURE);
    }

    multilog(health_args->log, LOG_INFO, "Health: Sending health data via udp to %s:%d.\n", health_args->health_udp_ip, health_args->health_udp_port);

    int quit = 0;

    while (!quit)
    {
        // Check quit status
        quit = get_quit();

        // Gather stats
        health_udp_data_s out_udp_data;

        // Version info
        out_udp_data.version_major = MWAX_DB2FITS_VERSION_MAJOR;
        out_udp_data.version_minor = MWAX_DB2FITS_VERSION_MINOR;
        out_udp_data.version_build = MWAX_DB2FITS_VERSION_PATCH;

        // Hostname
        strncpy(out_udp_data.hostname, health_args->hostname, HOST_NAME_LEN);

        // Time
        out_udp_data.start_time = start_time;
        out_udp_data.health_time = time(NULL);
        out_udp_data.up_time = difftime(out_udp_data.health_time, start_time);

        // Current work item
        if (get_health(&out_udp_data.status, &out_udp_data.obs_id, &out_udp_data.subobs_id) != EXIT_SUCCESS)
        {
            multilog(health_args->log, LOG_ERR, "Health: Error getting health data.\n");
            exit(EXIT_FAILURE);
        }

        //send the message
        if (sendto(sock, &out_udp_data, sizeof(health_udp_data_s), 0, (struct sockaddr *)&si_other, slen) == -1)
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

    return EXIT_SUCCESS;
}