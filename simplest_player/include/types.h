#pragma once

#include <cstdlib>

struct YuvData{
    uint8_t *pY_plane;
    int Y_pitch;
    uint8_t *pU_plane;
    int U_pitch;
    uint8_t *pV_plane;
    int V_pitch;
};