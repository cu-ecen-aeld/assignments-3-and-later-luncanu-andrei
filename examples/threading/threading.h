#include <stdbool.h>
#include <pthread.h>

/**
 * This structure should be dynamically allocated and passed as
 * an argument to your thread using pthread_create.
 * It should be returned by your thread so it can be freed by
 * the joiner thread.
 */
struct thread_data{
    /*
     * TODO: add other values your thread will need to manage
     * into this structure
     */
    
    // Pointer către mutex-ul pe care vrem să îl lock-ăm/unlock-ăm
    pthread_mutex_t *mutex;

    // Timpul de așteptare înainte de lock (în ms)
    int wait_to_obtain_ms;

    // Timpul cât ținem mutex-ul blocat (în ms)
    int wait_to_release_ms;

    /**
     * Set to true if the thread completed with success, false
     * if an error occurred.
     */
    bool thread_complete_success;
};


/**
* Start a thread which sleeps @param wait_to_obtain_ms number of milliseconds, then obtains the
* mutex in @param mutex, then holds for @param wait_to_release_ms milliseconds, then releases.
*/
bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms);
