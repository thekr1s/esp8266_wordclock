#include <string.h>
#include <lwip/sockets.h>

#include "esp_glue.h"
#include "AddressableLedStrip.h"
#include "controller.h"
#include "tetris_pieces.h"

#define UDP_POORT 21324

uint16_t _nrOfRows;
uint16_t _nrOfCols;
uint16_t _ledStripLen;

static void SetPixel(int index, uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
    // NOTE frames are stored as RGB show the white is missing, and dropped here
    int col;
    int row = index / _nrOfCols;
    if (row % 2 == 0) {
        col = _nrOfRows - (index % _nrOfRows);
    } else {
        col = index % _nrOfRows;
    }
    
    AlsSetLed(row, col, red, green, blue);
}

// https://github.com/Aircoookie/WLED/blob/main/wled00/udp.cpp
// UDP realtime: 1 warls 2 drgb 3 drgbw
static bool UDPFrameToPixelFrame(uint8_t* udpIn, size_t udpInLen) {
    

    if (udpInLen < 2) {
        return false; // UDP package will not contain any pixel info
    }

    if (udpIn[0] < 0 || udpIn[0] > 5) {
        return false; // Invalid protocol type
    }

    if (udpIn[1] == 0) {
        // Timeout is set to 0 so stop game
        ControllerGameSet(GAME_NONE);
        return false;
    }

    //warls - avoiding infinite "for" loop (unsigned underflow)    
    if ((udpIn[0] == 1) && (udpInLen > 5)) {
        for (size_t i = 2; i < udpInLen -3; i += 4) {
            SetPixel(udpIn[i], udpIn[i+1], udpIn[i+2], udpIn[i+3], 0);
        }
    } else if (udpIn[0] == 2) { //drgb
        uint16_t id = 0;
        for (size_t i = 2; i < udpInLen -2; i += 3) {
            SetPixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], 0);
            id++; if (id >= _ledStripLen) break;
        }
    } else if ((udpIn[0] == 3) && (udpInLen > 5)) { //drgbw - avoiding infinite "for" loop (unsigned underflow)
        uint16_t id = 0;
        for (size_t i = 2; i < udpInLen -3; i += 4) {
            SetPixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], udpIn[i+3]);
            id++; if (id >= _ledStripLen) break;
        }
    } else if (udpIn[0] == 4) { //dnrgb
        uint16_t id = ((udpIn[3] << 0) & 0xFF) + ((udpIn[2] << 8) & 0xFF00);
        for (size_t i = 4; i < udpInLen -2; i += 3) {
            if (id >= _ledStripLen) break;
            SetPixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], 0);
            id++;
        }
    } else if (udpIn[0] == 5) { //dnrgbw
        uint16_t id = ((udpIn[3] << 0) & 0xFF) + ((udpIn[2] << 8) & 0xFF00);
        for (size_t i = 4; i < udpInLen -2; i += 4) {
            if (id >= _ledStripLen) break;
            SetPixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], udpIn[i+3]);
            id++;
        }
    }
    return true;
}

void DoUdpRealtime() {
    struct sockaddr_in server_addr;
    int socket = -1;
    uint8_t payload[2000];
    size_t payloadLen;
    
    _nrOfRows = _displaySize[0];
    _nrOfCols = _displaySize[1];
    _ledStripLen = _displaySize[0] * _displaySize[1];
  
    // Set port and IP:
    memset(&server_addr, '0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_POORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket < 0) {
        printf("Error while creating Realtime UDP socket\n");
        return;
    }
    printf("Realtime UDP Socket created successfully\n");

    // Bind to the set port and IP:
    if(bind(socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Couldn't bind to the port\n");
        return;
    }
    printf("Done with binding Realtime UDP socket\n");

    AlsFill(0,0,0); 
    while (ControllerGameGet() == GAME_UDP_REALTIME) {
        if ((payloadLen = recv(socket, payload, sizeof(payload), 0)) < 0){
            printf("Couldn't receive\n");
            return;
        }
        
        if (UDPFrameToPixelFrame(payload, payloadLen)) {
            ControllerSet(CONTROLLER_NONE); //eatch frame is a contoller action
            AlsRefresh(ALSEFFECT_NONE);
        }
    }
    closesocket(socket);
}