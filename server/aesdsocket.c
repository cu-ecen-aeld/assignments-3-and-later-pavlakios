#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <stdbool.h>
#include <signal.h>
#include <arpa/inet.h>

#define PORT 9000
#define BUF_SIZE 65535

const char *tmp_file = "/var/tmp/aesdsocketdata";
volatile bool caught_signal = false;

static void signal_handler(int signal_number)
{
    if (signal_number == SIGTERM || signal_number == SIGINT)
        caught_signal = true;
}

static void set_signal_handler()
{
    struct sigaction new_action;

    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    if (sigaction(SIGTERM, &new_action, NULL) != 0) {
        syslog(LOG_ERR, "Error %d (%s) registering for SIGTERM", errno, strerror(errno));
        exit(-1);
    }
    if (sigaction(SIGINT, &new_action, NULL) != 0) {
        syslog(LOG_ERR, "Error %d (%s) registering for SIGINT", errno, strerror(errno));
        exit(-1);
    }
}

static void socket_service(int server_fd)
{
    char *buffer;
    int new_socket, fd;
    struct sockaddr address;
    ssize_t recv_len, read_len;
    int addrlen = 0;
    char ipstr[INET6_ADDRSTRLEN];

    fd = open(tmp_file, (O_RDWR | O_CREAT | O_TRUNC), (0664));
    if (fd == -1) {
        syslog(LOG_ERR, "Open file error: %s", strerror(errno));
        exit(-1);
    }

    buffer = (char *)malloc(BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);

    while (!caught_signal) {
        memset(&address, 0, sizeof(struct sockaddr));
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            if (caught_signal)
                break;
            syslog(LOG_ERR, "accept failed");
            exit(-1);
        }

        struct sockaddr_in *s = (struct sockaddr_in *)&address;
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));

        syslog(LOG_DEBUG, "Accepted connection from %s", ipstr);
        ssize_t index = 0;

        do {
            index += recv(new_socket, &buffer[index], BUF_SIZE, 0);
        } while (buffer[index-1] != '\n');
        recv_len = index;

        lseek(fd, 0, SEEK_END);
        write(fd, buffer, recv_len);
        memset(buffer, 0, recv_len);

        lseek(fd, 0, SEEK_SET);
        index = 0;
        while (true) {
            while (true) {
                read_len = read(fd, &buffer[index], 1);
                if (buffer[index] == '\n' || read_len <= 0)
                    break;
                index++;
            }
            if (buffer[index] == '\n') {
                read_len = index + 1;
                ssize_t send_len = send(new_socket, buffer, read_len, 0);
                memset(buffer, 0, send_len);
                index = 0;
            } else
                break;
        }
        close(new_socket);
        syslog(LOG_DEBUG, "Closed connection from %s", ipstr);
    }

    free(buffer);
    close(fd);
    remove(tmp_file);
}

int main(int argc, char **argv)
{
    int server_fd;
    pid_t pid;
    struct sockaddr_in address;
    int opt = 1;

    openlog(NULL, 0, LOG_USER);

    set_signal_handler();

    server_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        syslog(LOG_ERR, "failed to create socket: %d", server_fd);
        exit(-1);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        syslog(LOG_ERR, "setsockopt failed");
        exit(-1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        syslog(LOG_ERR, "bind failed");
        exit(-1);
    }

    if ((argc > 1) && (strcmp(argv[1], "-d") == 0)) {
        pid = fork ();
        if (pid == -1)
            return -1;
        else if (pid != 0) {
            syslog(LOG_DEBUG, "parent exits");
            closelog();
            exit(EXIT_SUCCESS);
        }
    }

    if (listen(server_fd, 3) < 0) {
        syslog(LOG_ERR, "listen failed");
        exit(-1);
    }

    socket_service(server_fd);

    syslog(LOG_DEBUG, "Caught signal, exiting");
    shutdown(server_fd, SHUT_RDWR);

    syslog(LOG_DEBUG, "Successfully cleaned up");
    closelog();

    return 0;
}