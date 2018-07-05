/**
 * @file global.c
 * @author Greg Sleap
 * @date 5 Jul 2018
 * @brief This is the code for anything global 
 *
 */
#include "global.h"
extern int g_quit;
extern pthread_mutex_t g_quit_mutex;

int initialise_quit()
{
    pthread_mutex_init(&g_quit_mutex, NULL);
}

int set_quit(int value)
{
    pthread_mutex_lock(&g_quit_mutex);
    g_quit = value;
    pthread_mutex_unlock(&g_quit_mutex);
}

int get_quit()
{
    int quit = 0;
    pthread_mutex_lock(&g_quit_mutex);
    quit = g_quit;
    pthread_mutex_unlock(&g_quit_mutex);

    return quit;
}

int destroy_quit()
{
    pthread_mutex_destroy(&g_quit_mutex);
}