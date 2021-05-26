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

pthread_mutex_t g_health_mutex;
health_thread_data_s g_health;

dada_db_s g_ctx;

/**
 * 
 *  @brief This creates the mutex used to ensure access to g_quit is thread-safe.
 *  @returns EXIT_SUCCESS on success. 
 */
int initialise_quit()
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
int set_quit(int value)
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
int get_quit()
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
int destroy_quit()
{
    pthread_mutex_destroy(&g_quit_mutex);
    return EXIT_SUCCESS;
}

/**
 * 
 *  @brief This creates the mutex used to ensure access to g_health is thread-safe.
 *  @returns EXIT_SUCCESS on success. 
 */
int initialise_health()
{
    pthread_mutex_init(&g_health_mutex, NULL);
    return EXIT_SUCCESS;
}

/**
 * 
 *  @brief A thread-safe way to set the values of g_health.
 *  @param[in] status. 
 *  @param[in] obs_id. 
 *  @param[in] subobs_id. 
 *  @returns EXIT_SUCCESS on success. 
 */
int set_health(int status, long obs_id, long subobs_id)
{
    pthread_mutex_lock(&g_health_mutex);
    g_health.status = status;
    g_health.obs_id = obs_id;
    g_health.subobs_id = subobs_id;
    pthread_mutex_unlock(&g_health_mutex);
    return EXIT_SUCCESS;
}

/**
 * 
 *  @brief A thread-safe way to return the values of g_health.
 *  @param[in out] status 
 *  @param[in out] obs_id 
 *  @param[in out] subobs_id
 *  @returns EXIT_SUCCESS on success. 
 */
int get_health(int *status, long *obs_id, long *subobs_id)
{
    pthread_mutex_lock(&g_health_mutex);
    *status = g_health.status;
    *obs_id = g_health.obs_id;
    *subobs_id = g_health.subobs_id;
    pthread_mutex_unlock(&g_health_mutex);

    return EXIT_SUCCESS;
}

/**
 * 
 *  @brief Destroys the g_health_mutex.
 *  @returns EXIT_SUCCESS on success. 
 */
int destroy_health()
{
    pthread_mutex_destroy(&g_health_mutex);
    return EXIT_SUCCESS;
}