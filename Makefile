# 输入参数
# target = common
# not_dir = iocp
# build_type = debug/release
# build_kind = lib/exe
# depends = 
# libs = 


solution_dir = ./
src_dir = $(solution_dir)src/
obj_dir = $(solution_dir)build/temp/
stdafx = $(solution_dir)src/include/stdafx.h

# 头文件
inc_third = ThirdParty/raknet/Source ThirdParty/googletest/include \
			ThirdParty/flatbuffers/include proto_file/output_cpp \
			ThirdParty/zlib ThirdParty/lua-5.3.4/src ThirdParty/LuaBridge/Source \
			ThirdParty/handy
inc_logic = src/common src/include src/rpc src/NetLib src/$(target)
inc_dir = $(patsubst %, -I$(solution_dir)%, $(inc_third) $(inc_logic))

# 源文件
ignore = $(patsubst %, -not -path '*/%/*', $(not_dir))
files_c = $(shell find $(src_dir)$(target) $(ignore) -name '*.c')
files_cpp = $(shell find $(src_dir)$(target) $(ignore) -name '*.cpp')
objs = $(subst $(src_dir),$(obj_dir), $(files_c:.c=.o)) $(subst $(src_dir),$(obj_dir), $(files_cpp:.cpp=.o))

# 库
LIBS = -L$(solution_dir)build -L$(solution_dir)ThirdParty $(libs)

# 编译选项
CFLAGS = -fPIC -fstack-protector-all -Wall -std=c++11 -Wno-error=format-security
CFLAGS += -Wno-sign-compare -Wno-deprecated
CXX = clang++
CC = clanag

ifeq ($(build_type),debug)
CFLAGS += -g3 -O0 -D_DEBUG
else
CFLAGS += -O2 -DNDEBUG
endif
ifeq ($(build_kind),exe)
output = $(solution_dir)build/$(target)
command = $(CXX) $(CFLAGS) $(inc_dir) $(objs) $(LIBS) -o $@
else
output = $(solution_dir)build/lib$(target).a
command = $(AR) -rcs $@ $(objs)
endif

# test:
# 	@echo $(CFLAGS)
# 	@echo $(files_cpp)
#	@echo $(objs)
# 	@echo $(inc_dir)

default: $(stdafx).gch $(output)

$(stdafx).gch:$(stdafx)
	$(CXX) $(CFLAGS) $(inc_dir) $< -o $@

$(output):$(objs) $(patsubst %, $(solution_dir)%, $(depends))
	$(command)

$(obj_dir)%.o: $(src_dir)%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(inc_dir) -MMD -MT $@ -MF $@.d -c $< -o $@
$(obj_dir)%.o: $(src_dir)%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(inc_dir) -MMD -MT $@ -MF $@.d -c $< -o $@

clean:
	rm -f $(objs) $(output)

-include $(addsuffix .d, $(objs))