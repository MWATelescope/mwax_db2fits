/**
 * @file health.c
 * @author Greg Sleap
 * @date 4 Jul 2018
 * @brief This is the code that sends health information to M&C via UDP
 *
 */
#include <arpa/inet.h>
#include <math.h>
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

    // initialise Weights in UDP struct
    for (int i = 0; i < NTILES_MAX; i++)
    {
        out_udp_data.weights_per_tile_x[i] = NAN;
        out_udp_data.weights_per_tile_y[i] = NAN;
    }

    // Version info
    out_udp_data.version_major = MWAX_DB2FITS_VERSION_MAJOR;
    out_udp_data.version_minor = MWAX_DB2FITS_VERSION_MINOR;
    out_udp_data.version_build = MWAX_DB2FITS_VERSION_PATCH;

    // Hostname
    strncpy(out_udp_data.hostname, health_args->hostname, HOST_NAME_LEN);

    // Time
    out_udp_data.start_time = start_time;

    int quit = quit_get();

    while (!quit)
    {
        out_udp_data.health_time = time(NULL);
        out_udp_data.up_time = difftime(out_udp_data.health_time, start_time);

        // Get current info
        pthread_mutex_lock(&g_health_manager_mutex);
        out_udp_data.status = g_health_manager.status;
        out_udp_data.obs_id = g_health_manager.obs_id;
        out_udp_data.subobs_id = g_health_manager.subobs_id;

        // We want to provide the health packet with an average
        // of the weights we've been accumulating,
        // but only if we have at least 1 set of weights

        // NOTE: before the first obs comes through ninputs will be 0!
        // and weights counter will be 0 too
        int ntiles = g_ctx.ninputs / 2;

        // If there has been at least one timestep with weights since the last health packet we will have ntiles>0 too...
        if (g_health_manager.weights_counter > 0)
        {
            // Store the average weight per tile and pol in the UDP health struct
            for (int tile = 0; tile < ntiles; tile++)
            {
                out_udp_data.weights_per_tile_x[tile] = g_health_manager.weights_per_tile_x[tile] / g_health_manager.weights_counter;
                out_udp_data.weights_per_tile_y[tile] = g_health_manager.weights_per_tile_y[tile] / g_health_manager.weights_counter;
            }

            // reset the weights now that we have got them ready to send via UDP
            g_health_manager.weights_counter = 0;
            for (int tile = 0; tile < ntiles; tile++)
            {
                g_health_manager.weights_per_tile_x[tile] = 0.0;
                g_health_manager.weights_per_tile_y[tile] = 0.0;
            }
        }
        else
        {
            // There has not been any new weights info since we sent the last health packet
            // So set the UDP weights array to NaNs
            for (int tile = 0; tile < NTILES_MAX; tile++)
            {
                out_udp_data.weights_per_tile_x[tile] = NAN;
                out_udp_data.weights_per_tile_y[tile] = NAN;
            }
        }

        pthread_mutex_unlock(&g_health_manager_mutex);
// debug dump of health
#ifdef DEBUG
        char health_debug_string[2048];
        snprintf(health_debug_string,
                 2048,
                 "v=%d.%d.%d h=%s start=%ld now=%ld up=%g st=%d obsid=%ld subobs=%ld",
                 out_udp_data.version_major,
                 out_udp_data.version_minor,
                 out_udp_data.version_build,
                 out_udp_data.hostname,
                 out_udp_data.start_time,
                 out_udp_data.health_time,
                 out_udp_data.up_time,
                 out_udp_data.status,
                 out_udp_data.obs_id,
                 out_udp_data.subobs_id);

        // If we have weights array initialised we'll dump it
        char xx_health_debug_string[2048] = "xx=";
        char yy_health_debug_string[2048] = "yy=";

        int tiles_to_dump = 1;
        if (ntiles > 0)
        {
            tiles_to_dump = ntiles;
        }

        for (int tile = 0; tile < tiles_to_dump; tile++)
        {
            char tmp[8];

            snprintf(tmp, 8, "%5.3f,", out_udp_data.weights_per_tile_x[tile]);
            strncat(xx_health_debug_string, tmp, 8);
            snprintf(tmp, 8, "%5.3f,", out_udp_data.weights_per_tile_y[tile]);
            strncat(yy_health_debug_string, tmp, 8);
        }

        // put all the log info together
        multilog(health_args->log, LOG_DEBUG, "health: %s %s %s. Resetting weights...\n", health_debug_string, xx_health_debug_string, yy_health_debug_string);
#endif

        // send the message
        if (sendto(sock, &out_udp_data, sizeof(health_udp_data_s), 0,
                   (struct sockaddr *)&groupSock,
                   sizeof(groupSock)) < 0)
        {
            multilog(health_args->log, LOG_ERR, "Health: Could not send health multicast datagram: call to sendto() failed.\n");
            exit(EXIT_FAILURE);
        }

        // Wait for 1 second
        sleep(HEALTH_SLEEP_SECONDS);

        // Check quit status
        quit = quit_get();
    }

    multilog(health_args->log, LOG_DEBUG, "Health: quit detected. Shutting down. Freeing weights memory...\n");

    // destroy the mutex and free the g_health_manager weights memory
    health_manager_destroy();

    multilog(health_args->log, LOG_DEBUG, "Health: Closing socket.\n");
    if (sock)
    {
        close(sock);
    }

    free(health_args->health_udp_interface_ip);

    multilog(health_args->log, LOG_INFO, "Health: Thread finished.\n");

    return EXIT_SUCCESS;
}