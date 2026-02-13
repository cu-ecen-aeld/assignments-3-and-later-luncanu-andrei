#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    // 1. Așteptăm înainte de a încerca să obținem mutex-ul
    // usleep primește microsecunde, deci înmulțim ms cu 1000
    usleep(thread_func_args->wait_to_obtain_ms * 1000);

    // 2. Obținem mutex-ul
    int rc = pthread_mutex_lock(thread_func_args->mutex);
    if (rc != 0) {
        ERROR_LOG("pthread_mutex_lock a eșuat cu codul %d", rc);
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    // 3. Așteptăm cât timp ținem mutex-ul blocat
    usleep(thread_func_args->wait_to_release_ms * 1000);

    // 4. Eliberăm mutex-ul
    rc = pthread_mutex_unlock(thread_func_args->mutex);
    if (rc != 0) {
        ERROR_LOG("pthread_mutex_unlock a eșuat cu codul %d", rc);
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    // Marcăm succesul și returnăm structura (pentru a putea fi eliberată ulterior cu join)
    thread_func_args->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    // Alocăm memorie pentru datele thread-ului
    // Folosim malloc pentru că datele trebuie să supraviețuiască după ce această funcție se termină
    struct thread_data* data = (struct thread_data*) malloc(sizeof(struct thread_data));
    if (data == NULL) {
        ERROR_LOG("Nu s-a putut aloca memorie pentru thread_data");
        return false;
    }

    // Setăm argumentele
    data->mutex = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    data->thread_complete_success = false;

    // Creăm thread-ul
    int rc = pthread_create(thread, NULL, threadfunc, data);
    if (rc != 0) {
        ERROR_LOG("pthread_create a eșuat cu codul %d", rc);
        free(data); // Curățăm memoria dacă thread-ul nu a putut fi creat
        return false;
    }

    return true;
}
