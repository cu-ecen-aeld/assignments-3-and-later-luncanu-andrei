#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define PORT 9000
#define DATA_FILE "/var/tmp/aesdsocketdata"

int server_fd = -1;

void handle_signal(int sig) {
    syslog(LOG_INFO, "Caught signal, exiting");
    if (server_fd != -1) close(server_fd);
    unlink(DATA_FILE);
    closelog();
    exit(0);
}

int main(int argc, char *argv[]) {
    openlog("aesdsocket", LOG_PID, LOG_USER);

    // Signal Handling
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Socket Creation
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) return -1;

    int opt = 1;
    // Set REUSEADDR so you can restart the server immediately without "Address already in use" errors
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = { 
        .sin_family = AF_INET, 
        .sin_port = htons(PORT), 
        .sin_addr.s_addr = INADDR_ANY 
    };

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        return -1;
    }

    // Daemon check: must happen AFTER bind
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        if (daemon(0, 0) == -1) {
            close(server_fd);
            return -1;
        }
    }

    if (listen(server_fd, 10) < 0) {
        close(server_fd);
        return -1;
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) continue;
        
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, INET_ADDRSTRLEN);
        syslog(LOG_INFO, "Accepted connection from %s", ip);

        // 1. Open file to APPEND the received packet
        FILE *fp_append = fopen(DATA_FILE, "a");
        if (!fp_append) {
            close(client_fd);
            continue;
        }

        char buf[1024];
        ssize_t bytes_recv;
        
        // Receive and Append Loop
        while ((bytes_recv = recv(client_fd, buf, sizeof(buf), 0)) > 0) {
            fwrite(buf, 1, bytes_recv, fp_append);
            // Check for newline to indicate end of packet
            if (memchr(buf, '\n', bytes_recv)) break;
        }

        // Close and flush the append stream so data is definitely on disk
        fclose(fp_append);

        // 2. Open file to READ full content back to client
        FILE *fp_read = fopen(DATA_FILE, "r");
        if (fp_read) {
            while ((bytes_recv = fread(buf, 1, sizeof(buf), fp_read)) > 0) {
                send(client_fd, buf, bytes_recv, 0);
            }
            fclose(fp_read);
        }

        close(client_fd);
        syslog(LOG_INFO, "Closed connection from %s", ip);
    }

    return 0;
}
