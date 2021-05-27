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
#include "utils.h"
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

    health_args->health_udp_interface_ip = malloc(sizeof(char) * IP_AS_STRING_LEN);

    // Get the ip for the outbound multicast interface for health
    multilog(health_args->log, LOG_INFO, "Health: Getting IP address for interface: %s.\n", health_args->health_udp_interface);

    if (get_ip_address_for_interface(health_args->health_udp_interface, health_args->health_udp_interface_ip) != EXIT_SUCCESS)
    {
        multilog(health_args->log, LOG_ERR, "Health: Could not get IP address for interface %s.\n", health_args->health_udp_interface);
        exit(EXIT_FAILURE);
    }

    multilog(health_args->log, LOG_INFO, "Health: IP address for interface %s is %s.\n", health_args->health_udp_interface, health_args->health_udp_interface_ip);

    // Initialise UDP socket
    int sock;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        multilog(health_args->log, LOG_ERR, "Health: Could not open a socket to health IP: call to socket() failed.\n");
        exit(EXIT_FAILURE);
    }

    //
    // Initialize the group sockaddr structure
    //
    struct sockaddr_in groupSock;
    memset((char *)&groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr(health_args->health_udp_ip);
    groupSock.sin_port = htons(health_args->health_udp_port);

    //
    // Set TTL
    //
    char ttl = 3;

    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
                   (char *)&ttl, sizeof(ttl)) < 0)
    {
        multilog(health_args->log, LOG_ERR, "Health: Could not set multicast TTL value.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    multilog(health_args->log, LOG_INFO, "Health: Multicast TTL is set for %s.\n", health_args->health_udp_interface_ip);

    //
    // Disable loopback so you do not receive your own datagrams.
    //
    char loopch = 0;

    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP,
                   (char *)&loopch, sizeof(loopch)) < 0)
    {
        multilog(health_args->log, LOG_ERR, "Health: Could not disable loopback during multicast init.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    multilog(health_args->log, LOG_INFO, "Health: Multicast Loopback is disabled.\n");

    //
    // Set local interface for outbound multicast datagrams.
    // The IP address specified must be associated with a local,
    // multicast-capable interface.
    //
    struct in_addr localInterface;

    localInterface.s_addr = inet_addr(health_args->health_udp_interface_ip);
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF,
                   (char *)&localInterface,
                   sizeof(localInterface)) < 0)
    {
        multilog(health_args->log, LOG_ERR, "Health: Could not configure local interface %s for multicast.\n", health_args->health_udp_interface_ip);
        close(sock);
        exit(EXIT_FAILURE);
    }

    multilog(health_args->log, LOG_INFO, "Health: Multicast configured successfully on %s (%s) going to %s:%d.\n", health_args->health_udp_interface, health_args->health_udp_interface_ip, health_args->health_udp_ip, health_args->health_udp_port);

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

    int quit = 0;

    while (!quit)
    {
        // Check quit status
        quit = get_quit();

        out_udp_data.health_time = time(NULL);
        out_udp_data.up_time = difftime(out_udp_data.health_time, start_time);

        // Current work item
        if (get_health(&out_udp_data.status, &out_udp_data.obs_id, &out_udp_data.subobs_id) != EXIT_SUCCESS)
        {
            multilog(health_args->log, LOG_ERR, "Health: Error getting health data.\n");
            exit(EXIT_FAILURE);
        }

        //send the message
        if (sendto(sock, &out_udp_data, sizeof(health_udp_data_s), 0,
                   (struct sockaddr *)&groupSock,
                   sizeof(groupSock)) < 0)
        {
            multilog(health_args->log, LOG_ERR, "Health: Could not send health multicast datagram: call to sendto() failed.\n");
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