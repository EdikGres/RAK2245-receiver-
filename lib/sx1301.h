//
// Created by root on 04.03.23.
//

#ifndef TESTPI_SX1301_H
#define TESTPI_SX1301_H

#include <utility>

#include "../json-3.11.2/single_include/nlohmann/json.hpp"
extern "C" {
#include "loragw_hal.h"
}

/*
 * singleton sx1301
 */

class sx1301 {
private:
    static sx1301* instance;
    struct lgw_conf_board_s lgwconf;
    struct lgw_conf_rxrf_s lgw_radios[2];
    struct lgw_conf_rxif_s lgw_channels[8];
    struct lgw_conf_rxif_s lgw_LoRa;
    struct lgw_conf_rxif_s lgw_FSK;

    sx1301(nlohmann::json data);

    sx1301(const sx1301&) = delete;
    sx1301& operator=(sx1301&) = delete;

    void parseJSON(nlohmann::json data);

public:
    ~sx1301();
    static sx1301* getInstance(nlohmann::json data) {
        if(!instance)
            instance = new sx1301(std::move(data));
        return instance;
    }

};


#endif //TESTPI_SX1301_H
