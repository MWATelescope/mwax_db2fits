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
extern int g_quit;
extern pthread_mutex_t g_quit_mutex;

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