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

/**
 * 
 *  @brief Collects the health stats and populates the heath_data structure. This is what gets sent out in binary form via UDP.
 *  @param[in] health_data Pointer to the health_data_s struct to be populated.
 *  @param[in] header_block Pointer to the header block buffer structure, where we get stats from.
 *  @param[in] data_block Pointer to the data block buffer structure, where we get stats from.
 *  @returns EXIT_SUCCESS on success, or EXIT_FAILURE if there was an error. 
 */
int collect_buffer_stats(health_data_s *health_data, ipcbuf_t *header_block, ipcbuf_t *data_block)
{
    // Get stats
    health_data->hdr_bufsz = ipcbuf_get_bufsz(header_block);
    
    health_data->hdr_nbufs = ipcbuf_get_nbufs(header_block);
    health_data->hdr_bytes = health_data->hdr_nbufs * health_data->hdr_bufsz;

    health_data->data_bufsz = ipcbuf_get_bufsz(data_block);
    health_data->data_nbufs = ipcbuf_get_nbufs(data_block);
    health_data->data_bytes = health_data->data_nbufs * health_data->data_bufsz;
    health_data->n_readers = ipcbuf_get_nreaders(data_block);

    health_data->hdr_bufs_written = ipcbuf_get_write_count(header_block);
    health_data->hdr_bufs_read = ipcbuf_get_read_count(header_block);
    health_data->hdr_full_bufs = ipcbuf_get_nfull(header_block);
    health_data->hdr_clear_bufs = ipcbuf_get_nclear(header_block);
    health_data->hdr_available_bufs = (health_data->hdr_nbufs - health_data->hdr_full_bufs);

    int i_reader = 0;
    health_data->data_bufs_written = ipcbuf_get_write_count (data_block);
    health_data->data_bufs_read = ipcbuf_get_read_count_iread (data_block, i_reader);
    health_data->data_full_bufs = ipcbuf_get_nfull_iread (data_block, i_reader);
    health_data->data_clear_bufs = ipcbuf_get_nclear_iread (data_block, i_reader);
    health_data->data_available_bufs = (health_data->data_nbufs - health_data->data_full_bufs);   

    return EXIT_SUCCESS;
}

/**
 * 
 *  @brief This is the main health thread function to send health data for this process via UDP.
 *  @param[in] args Pointer to the arguments that main() passes to this function.
 *  @returns void. 
 */
void* health_thread_fn(void *args)
{
    health_thread_args_s *health_args = (health_thread_args_s*)args;

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
    int slen=sizeof(struct sockaddr_in);
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;    

    si_other.sin_port = htons(health_args->health_udp_port);    

    if (inet_aton(health_args->health_udp_ip , &si_other.sin_addr) == 0) 
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

        // Gather stats from buffers
        health_data_s data;
        data.status = health_args->status;
        collect_buffer_stats(&data, health_args->header_block, health_args->data_block);        

        //send the message        
        if (sendto(sock, &data, sizeof(health_data_s), 0, (struct sockaddr *) &si_other, slen) == -1)
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