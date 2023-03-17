#pragma once

struct RecordPacket {
    float delay = 0;
    float duration = 0;
    char prefixPath[16] = {};
    int desiredFPS = 15;
    int record = 1;
    int close = 0;
};