#include "stdafx.h"
#include "config_net.h"
#include "Buffer/NetPack.h"

std::vector<NetMeta> NetMeta::_table;
const NetMeta* NetMeta::G_Local_Meta = NULL;

const NetMeta* NetMeta::GetMeta(std::string module, int svrId /* = -1 */) {
    for (auto& it : _table) {
        if (it.module == module && (svrId < 0 || it.svr_id == svrId)) {
            return &it;
        }
    }
    return NULL;
}
void NetMeta::AddMeta(const NetMeta& meta)
{
    for (auto& it : _table) {
        if (it.module == meta.module && it.svr_id == meta.svr_id) {
            it = meta;
            return;
        }
    }
    _table.push_back(meta);
}
void NetMeta::DelMeta(std::string module, int svrId)
{
    for (auto& it : _table) {
        if (it.module == module && it.svr_id == svrId) {
            it.is_closed = true;
            return;
        }
    }
}

void NetMeta::DataToBuf(NetPack& buf) const
{
    buf.WriteString(module);
    buf.WriteInt32(svr_id);
    buf.WriteString(svr_name);
    buf.WriteString(version);
    buf.WriteString(ip);
    buf.WriteString(out_ip);
    buf.WriteUInt16(tcp_port);
    buf.WriteUInt16(http_port);
    buf.WriteInt32(max_conn);
    uint8 length = 1;
    buf.WriteUInt8(length);
    for (uint8 i = 0; i < length; ++i) {
        buf.WriteString("cross"); //TODO:zhoumf:同go那边格式统一
    }
}
void NetMeta::BufToData(NetPack& buf)
{
    module = buf.ReadString();
    svr_id = buf.ReadInt32();
    svr_name = buf.ReadString();
    version = buf.ReadString();
    ip = buf.ReadString();
    out_ip = buf.ReadString();
    tcp_port = buf.ReadUInt16();
    http_port = buf.ReadUInt16();
    max_conn = buf.ReadInt32();
    uint8 length = buf.ReadUInt8();
    for (uint8 i = 0; i < length; ++i) {
        buf.ReadString();
    }
}