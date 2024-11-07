#ifndef DEFS_H
#define DEFS_H

#include <QString>

// Centralized list of parameters

const int SCENE_WIDTH = 800;        // Scene dimensions
const int SCENE_HEIGHT = 600;

const int PLAYER_WIDTH = 30;        // Player dimensions
const int PLAYER_HEIGHT = 30;

// Time interval between frames in milliseconds
// 24 is correct!!!!
const int FRAME_RATE = 24;
//const int FRAME_RATE = 100;

const QString SERVER_IP = "172.21.216.11";
const quint16 PORT = 5678;

#endif // DEFS_H
