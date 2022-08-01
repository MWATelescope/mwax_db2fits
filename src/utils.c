/**
 * @file utils.c
 * @author Greg Sleap
 * @date 21 May 2018
 * @brief This is the code that handles misc functions like time and formatting
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <ifaddrs.h>

#include "global.h"

/**
 *
 *  @brief This populates the out_timeinfo structure with the current local time.
 *  @param[out] out_timeinfo Pointer to the pointer where we will create the struct tm object.
 *  @returns EXIT_SUCCESS on success.
 */
int get_time_struct(struct tm **out_timeinfo)
{
    time_t rawtime;

    time(&rawtime);
    *out_timeinfo = localtime(&rawtime);

    int month = (*out_timeinfo)->tm_mon + 1;
    int year = (*out_timeinfo)->tm_year + 1900;

    (*out_timeinfo)->tm_mon = month;
    (*out_timeinfo)->tm_year = year;

    return EXIT_SUCCESS;
}

/**
 *
 *  @brief This populates the timestring with the current time.
 *  @param[out] timestring Pointer to the string to be populated.
 *  @returns EXIT_SUCCESS on success,.
 */
int get_time_string_for_fits(char *timestring)
{
    struct tm *timeinfo;
    get_time_struct(&timeinfo);

    sprintf(timestring, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    return EXIT_SUCCESS;
}

/**
 *
 *  @brief This populates the timestring with the current time, but formatted for use in a log.
 *  @param[out] timestring Pointer to the string to be populated.
 *  @returns EXIT_SUCCESS on success.
 */
int get_time_string_for_log(char *timestring)
{
    struct tm *timeinfo;
    get_time_struct(&timeinfo);

    sprintf(timestring, "%4d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    return EXIT_SUCCESS;
}

/**
 *
 *  @brief This attempts to get the IPv4 address of a named network interface.
 *  @param[in] interface A char* containing the interface name- e.g. eth0.
 *  @param[out] out_ip_address an allocated char* buffer containing the IP v4 address- this should be min of 15 + 1 characters
 *  @returns EXIT_SUCCESS on success or any other value on failure.
 */
int get_ip_address_for_interface(const char *interface, char *out_ip_address)
{
    struct ifaddrs *id;

    // get a struct containing info about all interfaces
    if (getifaddrs(&id) == 0)
    {
        struct ifaddrs *iface_item = id;

        // loop through all interfaces
        while (iface_item)
        {
            // Find the correct interface
            if (iface_item->ifa_addr && iface_item->ifa_addr->sa_family == AF_INET && strcmp(iface_item->ifa_name, interface) == 0)
            {
                struct sockaddr_in *pAddr = (struct sockaddr_in *)iface_item->ifa_addr;

                /* copy result into output buffer */
                sprintf(out_ip_address, "%s", inet_ntoa(pAddr->sin_addr));

                // Free the memory used by getifaddrs
                freeifaddrs(id);

                return EXIT_SUCCESS;
            }

            // Get next interface
            iface_item = iface_item->ifa_next;
        }

        freeifaddrs(id);

        return EXIT_FAILURE;
    }
    else
    {
        return EXIT_FAILURE;
    }
}