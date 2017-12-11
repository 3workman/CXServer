#pragma once

const uint MAX_PLAYER_COUNT = 50;   //玩家数量上限

const uint MAX_ROBOT_COUNT = 50;    //机器人数量

enum RoleType : int
{
    // 不要改变顺序
    ROLE_HERO = 0,
    ROLE_BOSS = 1,
    ROLE_MINION = 2,
};