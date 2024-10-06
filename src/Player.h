/*
 * Player.h
 *
 * Created by miles
*/

#pragma once


class Player {
public:
    Player() {}
    Player(const Player&) = delete;
    Player(const Player&&) = delete;
    void operator =(const Player&) = delete;
public:
    Vector3 pos = {5, 10, -8};
    Vector3 camView = {-40, 0, 40};
    Vector3 vel = {0, 0, 0};
    Vector3 up = {0, 1, 0};
    Vector3 propel = {0, 0, 0};
    float scale = 8;
    bool brake = false;
    uint32_t followers;
};
