#include "lib/pch.h"
#include "lib/p2p_thread.h"
#include "lib/protocol.h"
#include "lib/client.h"

#define MIN_PORT 5679
#define SERVER_PORT 5678

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    in_port_t port = atoi(argv[1]);
    if (port < MIN_PORT) {
        fprintf(stderr, "Error: Invalid port number.\n");
        exit(1);
    }

    Client* client = client_create(port);

    client_start_p2p(client, p2p_server_function);

    while (client->state != STATE_SHUTTING_DOWN) {
        switch (client->state) {
            case STATE_STARTING_P2P:
                // Waiting for P2P server to start
                break;

            case STATE_CONNECTING:
                client_connect_to_server(client, "127.0.0.1", SERVER_PORT);
                client_send_server_hello(client);

                client_update_state(client, STATE_IDLE);
                break;

            case STATE_IDLE:
                // Handle idle state
                break;

            case STATE_DISCONNECTING:
                server_stop();
                break;

            case STATE_SHUTTING_DOWN:
                break;
        }
    }
    
    // Sending initial hello message


   /*

    // Set up signal handlers
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    state = STATE_IDLE;

    while (state != STATE_DISCONNECTING) {
        read_fds = master_set;
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            fprintf(stderr, "Error: select() failed.\n");
            close(server_fd);
            exit(1);
        }

        if (activity == 0) {
            continue; 
        }

        for (int fd = 0; fd < max_fd; fd++) {
            if (!FD_ISSET(fd, &read_fds)) {
                continue;
            }

            if (fd == server_fd) {

            }
            else if (fd == STDIN_FILENO) {

            }
            else {

            }
        }
    }

    close(server_fd);
    */

    
    return 0;
}