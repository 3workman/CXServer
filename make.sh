#!/bin/sh

cd ../GoServer/bin
./generate
cd -

make $1 target='common' build_type='debug' build_kind='lib' \
        depends='ThirdParty/liblua.a ThirdParty/libz.a'

make $1 target='NetLib' not_dir='iocp handy' build_type='debug' build_kind='lib' \
        depends='ThirdParty/libRakNet.a ThirdParty/libhandy.a ThirdParty/libevent.a'

make $1 target='svr_battle' build_type='debug' build_kind='exe' \
        depends='build/libcommon.a build/libNetLib.a' \
        libs='-lpthread -lcommon -lNetLib -lgtest -lz -llua -lRakNet -levent'