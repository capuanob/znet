#define ZN_IMPLEMENTATION
#include <stdint.h> // uint8_t
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "../znet.h"

#define send_string(str) str, sizeof(str) - 1
#define PORT 12345
#define MAX_BUFFER_SIZE 4096

zn_State *S; // Event loop handler
zn_Timer *timer; // Timer handler
zn_Tcp *tcp; // Client handler
zn_Accept *a; // Service handler

typedef struct FuzzerData {
    uint8_t data[MAX_BUFFER_SIZE];
    size_t size;
} FuzzerData;

void on_accept(void *ud, zn_Accept *accept, unsigned err, zn_Tcp *tcp) {
    (void)ud;

    if (err != ZN_OK) {
        fprintf(stderr, "Error encountered when accepting new connection!\n");
        return;
    }

    FuzzerData *server_fuzzer_data = (FuzzerData*) malloc(sizeof(FuzzerData));
    zn_recv(tcp, (char*) server_fuzzer_data->data, MAX_BUFFER_SIZE, NULL, &server_fuzzer_data);
    zn_deltcp(tcp); // Close connection
    free(server_fuzzer_data);
}

void initialize_server() {
    S = zn_newstate();

    if (fork() == 0) {
        // Child should initialize the server
        if (S == NULL) {
            fprintf(stderr, "Unable to create a znet server!\n");
            exit(2);
        }

        printf("Server initialized!\n");
        a = zn_newaccept(S, 0);

        if (zn_listen(a, "127.0.0.1", PORT) != ZN_OK) {
            fprintf(stderr, "Unable to listen to ip:port!");
            exit(3);
        }

        zn_accept(a, on_accept, NULL);
        zn_run(S, ZN_RUN_LOOP);
        exit(0);
    } else {
        // Parent should wait for server to be initialized before continuing
        sleep(3);
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > MAX_BUFFER_SIZE) return 0;

    // Populate fuzzer object with data we would like to send
    FuzzerData *client_fuzzer_data = (FuzzerData*) malloc(sizeof(FuzzerData));
    memcpy(client_fuzzer_data->data, data, size);
    client_fuzzer_data->size = size;

    tcp = zn_newtcp(S);
    zn_connect(tcp, "127.0.0.1", PORT, 0, NULL, client_fuzzer_data);
    zn_deltcp(tcp);
    free(client_fuzzer_data);

    return 0;
}

int LLVMFuzzerInitialize(int *argc, char ***argv) {
    initialize_server();
    return 0;
}
