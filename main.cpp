#include <iostream>
#include "lib/sx1301.h"
#include "json-3.11.2/single_include/nlohmann/json.hpp"
#include <fstream>
#include <csignal>
#include <ctime>


using json = nlohmann::json;

static sx1301 *s;

void signal_handler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received." << std::endl;

    delete (s);
    exit(signum);
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGTERM, signal_handler);


    std::ifstream f("../config_sx1301.json");
    if (!f.is_open()) {
        std::cout << "no such file" << std::endl;
        exit(-1);
    }
    json data = json::parse(f);

    s = sx1301::getInstance(data);

    struct lgw_pkt_rx_s rxpkt[16]; /* array containing up to 16 inbound packets metadata */
    struct lgw_pkt_rx_s *p; /* pointer on a RX packet */
    struct timespec sleep_time = {0, 2000000};

    int nb_pkt = 0;

    while (true) {
        nb_pkt = lgw_receive(16, rxpkt);
        if (nb_pkt == LGW_HAL_ERROR) {
            printf("ERROR: failed packet fetch, exiting\n");
            return EXIT_FAILURE;
        } else if (nb_pkt == 0) {
            clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_time, NULL); /* wait a short time if no packets */
        } else {
        }
        for (int i = 0; i < nb_pkt; ++i) {
            p = &rxpkt[i];
            printf("Packet received!!!\n");
            for (int j = 0; j < p->size; ++j) {
                printf("%04x", p->payload[j]);
            }
            printf("\n");
        }
    }


    f.close();


    delete (s);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
