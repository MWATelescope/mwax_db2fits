/**
 * @file global.c
 * @author Greg Sleap
 * @date 5 Jul 2018
 * @brief This is the code for anything global
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"

pthread_mutex_t g_quit_mutex;
int g_quit = 0;

pthread_mutex_t g_health_manager_mutex;
health_thread_data_s g_health_manager;

dada_db_s g_ctx;

/**
 *
 *  @brief This creates the mutex used to ensure access to g_quit is thread-safe.
 *  @returns EXIT_SUCCESS on success.
 */
int quit_init()
{
    pthread_mutex_init(&g_quit_mutex, NULL);
    return EXIT_SUCCESS;
}

/**
 *
 *  @brief A thread-safe way to set the value of g_quit.
 *  @param[in] Value to set g_quit to.
 *  @returns EXIT_SUCCESS on success.
 */
int quit_set(int value)
{
    pthread_mutex_lock(&g_quit_mutex);
    g_quit = value;
    pthread_mutex_unlock(&g_quit_mutex);
    return EXIT_SUCCESS;
}

/**
 *
 *  @brief A thread-safe way to return the value of g_quit.
 *  @returns Value of g_quit.
 */
int quit_get()
{
    int quit = 0;
    pthread_mutex_lock(&g_quit_mutex);
    quit = g_quit;
    pthread_mutex_unlock(&g_quit_mutex);

    return quit;
}

/**
 *
 *  @brief Destroys the g_quit_mutex.
 *  @returns EXIT_SUCCESS on success.
 */
int quit_destroy()
{
    pthread_mutex_destroy(&g_quit_mutex);
    return EXIT_SUCCESS;
}

///
/// NOTE: the "health_manager" methods below are for the main program to manipulate the g_health_manager struct which will eventually be passed to the health thread to create a UDP health packet.
///

/**
 *
 *  @brief This creates the mutex used to ensure access to g_health_manager is thread-safe. It also inits the other attributes. g_health_manager is the global var which keeps info which will eventually be used to send health info.
 *  @returns EXIT_SUCCESS on success.
 */
int health_manager_init()
{
    pthread_mutex_init(&g_health_manager_mutex, NULL);

    pthread_mutex_lock(&g_health_manager_mutex);
    g_health_manager.status = STATUS_RUNNING;
    g_health_manager.obs_id = 0;
    g_health_manager.subobs_id = 0;
    g_health_manager.weights_counter = 0;
    g_health_manager.weights_per_tile_xx = NULL;
    g_health_manager.weights_per_tile_yy = NULL;
    pthread_mutex_unlock(&g_health_manager_mutex);

    return EXIT_SUCCESS;
}

/**
 *
 *  @brief A thread-safe way to set the values of g_health_manager.
 *  @param[in] status.
 *  @param[in] obs_id.
 *  @param[in] subobs_id.
 *  @returns EXIT_SUCCESS on success.
 */
int health_manager_set_info(int status, long obs_id, long subobs_id)
{
    pthread_mutex_lock(&g_health_manager_mutex);
    g_health_manager.status = status;
    g_health_manager.obs_id = obs_id;
    g_health_manager.subobs_id = subobs_id;
    pthread_mutex_unlock(&g_health_manager_mutex);
    return EXIT_SUCCESS;
}

/**
 *
 *  @brief A thread-safe way to reset the weights values of g_health_manager.
 *  @returns EXIT_SUCCESS on success.
 */
int health_manager_reset_health_weights_info()
{
    pthread_mutex_lock(&g_health_manager_mutex);
    g_health_manager.weights_counter = 0;
    if (g_health_manager.weights_per_tile_xx != NULL)
    {
        for (int tile = 0; tile < g_ctx.ninputs / 2; tile++)
        {
            g_health_manager.weights_per_tile_xx[tile] = 0;
            g_health_manager.weights_per_tile_yy[tile] = 0;
        }
    }
    pthread_mutex_unlock(&g_health_manager_mutex);
    return EXIT_SUCCESS;
}

/**
 *
 *  @brief A thread-safe way to set the weights values of g_health_manager.
 *  @param[in] buffer - pointer to buffer containing baseline weights
 *  @returns EXIT_SUCCESS on success.
 */
int health_manager_set_weights_info(float *buffer)
{
    pthread_mutex_lock(&g_health_manager_mutex);
    // Update the weights arrays and counter

    // Check if we have initialised the array yet.
    // We only know the number of tiles we have at this point
    if (g_health_manager.weights_per_tile_xx == NULL)
    {
        // Malloc the arrays
        g_health_manager.weights_per_tile_xx = calloc(g_ctx.ninputs / 2, sizeof(float));
        g_health_manager.weights_per_tile_yy = calloc(g_ctx.ninputs / 2, sizeof(float));
    }

    g_health_manager.weights_counter++;

    int baseline = 0;

    for (int i = 0; i < g_ctx.ninputs / 2; i++)
    {
        for (int j = 0; j < i; j++)
        {
            baseline++;
            int xx_index = baseline * 4; // There are 4 pols per baseline in the weights provided to us
            int yy_index = xx_index + 3; // Pols are ordered xx, xy, yx, yy, so add 3 to the xx index.

            g_health_manager.weights_per_tile_xx[i] += buffer[xx_index];
            g_health_manager.weights_per_tile_yy[j] += buffer[yy_index];
        }
    }

    pthread_mutex_unlock(&g_health_manager_mutex);
    return EXIT_SUCCESS;
}

/**
 *
 *  @brief Destroys the g_health_manager_mutex.
 *  @returns EXIT_SUCCESS on success.
 */
int health_manager_destroy()
{
    pthread_mutex_destroy(&g_health_manager_mutex);

    if (g_health_manager.weights_per_tile_xx != NULL)
    {
        free(g_health_manager.weights_per_tile_xx);
        free(g_health_manager.weights_per_tile_yy);
    }
    return EXIT_SUCCESS;
}