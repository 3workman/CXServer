#include "stdafx.h"
#include "trigger.h"
#include "tool\GameApi.h"

static const Trigger::TriggerFunc g_handler[] = {
    &Trigger::IsUpLevel,
    &Trigger::IsDuringTime,
};
STATIC_ASSERT_ARRAY_LENGTH(g_handler, Trigger::MAX_ENUM);

Trigger::Trigger()
{
    // 读表
}
bool Trigger::Check(Player* player, const int triggerId)
{
    auto it = m_TriggerLst.find(triggerId);
    if (it != m_TriggerLst.end())
    {
        const TriggerTable& data = it->second;
        return (this->*g_handler[data.type])(player, data.val1, data.val2);
    }
    return false;
}
bool Trigger::Check(Player* player, const std::vector<uint16>& triggerIds)
{
    for (auto it : triggerIds)
    {
        if (!Check(player, it)) return false;
    }
    return true;
}


//----------------------------------------------------------
//各类判断函数
bool Trigger::IsUpLevel(Player* player, int64 val1, int64 val2)
{
    if (player)
    {
        //return player.GetLevel() >= val1;
    }
    return false;
}
bool Trigger::IsDuringTime(Player* player, int64 val1, int64 val2)
{
    int64 timeNow = GameApi::TimeNowSec();

    return timeNow >= GameApi::ParseTime(val1) && timeNow < GameApi::ParseTime(val2);
}