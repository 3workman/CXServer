#pragma once
#include "Csv/CSVparser.hpp"
#include "config_net.h"

inline void LoadAllCsv()
{
    csv::LoadCsv(NetMeta::_table, "conf_net.csv");
}