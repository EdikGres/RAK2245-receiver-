//
// Created by root on 04.03.23.
//

#include "sx1301.h"
#include <iostream>
#include <utility>

sx1301::sx1301(nlohmann::json data) {
    parseJSON(std::move(data));
    //gateway
    lgw_board_setconf(this->lgwconf);
    //radios
    if (lgw_rxrf_setconf(0, this->lgw_radios[0]) != LGW_HAL_SUCCESS) {
        std::cout << "ERROR: invalid configuration for radio " << 0 << std::endl;
        exit(-1);
    }
    if (lgw_rxrf_setconf(1, this->lgw_radios[1]) != LGW_HAL_SUCCESS) {
        std::cout << "ERROR: invalid configuration for radio " << 1 << std::endl;
        exit(-1);
    }
    //mulstiSF
    for (int i = 0; i < LGW_MULTI_NB; ++i) {
        if (lgw_rxif_setconf(i, this->lgw_channels[i]) != LGW_HAL_SUCCESS) {
            std::cout << "ERROR: invalid configuration for Lora multi-SF channel %i" << std::endl;
            exit(-1);
        }
    }
    //LoRa
    if (lgw_rxif_setconf(8, this->lgw_LoRa) != LGW_HAL_SUCCESS) {
        std::cout << "ERROR: invalid configuration for Lora standard channel" << std::endl;
        exit(-1);
    }
    //FSK
    if (lgw_rxif_setconf(9, this->lgw_FSK) != LGW_HAL_SUCCESS) {
        std::cout << "ERROR: invalid configuration for FSK channel" << std::endl;
        exit(-1);
    }

    //start lgw
    int i = lgw_start();
    if (i == LGW_HAL_SUCCESS) {
        std::cout << "INFO: concentrator started, packet can be sent" << std::endl;
    } else {
        std::cout << "ERROR: failed to start the concentrator" << std::endl;
        exit(-1);
    }

}

sx1301::~sx1301() {
    lgw_stop();
    std::cout << "LGW stopped" << std::endl;
}

sx1301 *sx1301::instance = nullptr;

void sx1301::parseJSON(nlohmann::json data) {
    if (data.empty()) {
        std::cout << "json is empty!" << std::endl;
        exit(-1);
    }
    if (data["sx1301"].empty()) {
        std::cout << "sx1301 is empty!" << std::endl;
        exit(-1);
    }

    //lgw parse
    this->lgwconf.lorawan_public = (bool)data["sx1301"]["lorawan_public"];

    if (data["sx1301"]["clock_source"] == 1)
        this->lgwconf.clksrc = 1;
    else
        this->lgwconf.clksrc = 0;

    //radios parse
    std::string radio_set = "radio_x";
    for (int i = 0; i < LGW_RF_CHAIN_NB; ++i) {
        radio_set[radio_set.size() - 1] = 0x30 + i;
        if (data["sx1301"][radio_set]["enable"])
            this->lgw_radios[i].enable = true;
        else
            this->lgw_radios[i].enable = false;
        if ((std::string) data["sx1301"][radio_set]["type"] == "SX1257")
            this->lgw_radios[i].type = LGW_RADIO_TYPE_SX1257;
        else if ((std::string) data["sx1301"][radio_set]["type"] == "SX1257")
            this->lgw_radios[i].type = LGW_RADIO_TYPE_SX1255;
        else {
            std::cout << "Error radio_" << i << " type" << std::endl;
            exit(-1);
        }
        this->lgw_radios[i].freq_hz = (uint32_t) data["sx1301"][radio_set]["freq"];
        this->lgw_radios[i].rssi_offset = (float) data["sx1301"][radio_set]["rssi_offset"];
        if (data["sx1301"][radio_set]["tx_enable"])
            this->lgw_radios[i].tx_enable = true;
        else
            this->lgw_radios[i].tx_enable = false;
        this->lgw_radios[i].tx_notch_freq = (uint32_t) data["sx1301"][radio_set]["tx_notch_freq"];

    }

    //channels multiSF parse
    std::string multiSF_set = "chan_multiSF_x";
    for (int i = 0; i < LGW_MULTI_NB; ++i) {
        multiSF_set[multiSF_set.size() - 1] = 0x30 + i;
        if (data["sx1301"][multiSF_set]["enable"]) {
            this->lgw_channels[i].enable = true;
            this->lgw_channels[i].rf_chain = (uint8_t) data["sx1301"][multiSF_set]["radio"];
            if (!((int32_t) data["sx1301"][multiSF_set]["if"] < -400000 ||
                  (int32_t) data["sx1301"][multiSF_set]["if"] > 400000))
                this->lgw_channels[i].freq_hz = (int32_t) data["sx1301"][multiSF_set]["if"];
            else {
                std::cout << "error 'if' frequency multiSF more than 400000!" << std::endl;
                exit(-1);
            }
        } else {
            this->lgw_channels[i].enable = false;
        }
    }

    //channel LoRa std parse
    if (data["sx1301"]["chan_LoRa_std"]["enable"]) {
        this->lgw_LoRa.enable = true;
        this->lgw_LoRa.rf_chain = (uint8_t) data["sx1301"]["chan_LoRa_std"]["radio"];
        if (!((int32_t) data["sx1301"]["chan_LoRa_std"]["if"] < -400000 ||
              (int32_t) data["sx1301"]["chan_LoRa_std"]["if"] > 400000))
            this->lgw_LoRa.freq_hz = (int32_t) data["sx1301"]["chan_LoRa_std"]["if"];
        else {
            std::cout << "error 'if' frequency LoRa-std more than 400000!" << std::endl;
            exit(-1);
        }
        switch ((int) data["sx1301"]["chan_LoRa_std"]["bw"]) {
            case 500000:
                this->lgw_LoRa.bandwidth = BW_500KHZ;
                break;
            case 250000:
                this->lgw_LoRa.bandwidth = BW_250KHZ;
                break;
            case 125000:
                this->lgw_LoRa.bandwidth = BW_125KHZ;
                break;
            default:
                this->lgw_LoRa.bandwidth = BW_UNDEFINED;
                break;
        }
        switch ((int) data["sx1301"]["chan_LoRa_std"]["sf"]) {
            case 7:
                this->lgw_LoRa.datarate = DR_LORA_SF7;
                break;
            case 8:
                this->lgw_LoRa.datarate = DR_LORA_SF8;
                break;
            case 9:
                this->lgw_LoRa.datarate = DR_LORA_SF9;
                break;
            case 10:
                this->lgw_LoRa.datarate = DR_LORA_SF10;
                break;
            case 11:
                this->lgw_LoRa.datarate = DR_LORA_SF11;
                break;
            case 12:
                this->lgw_LoRa.datarate = DR_LORA_SF12;
                break;
            default:
                this->lgw_LoRa.datarate = DR_UNDEFINED;
        }
    } else {
        this->lgw_LoRa.enable = false;
    }

    //TODO: for FSK parse

}
