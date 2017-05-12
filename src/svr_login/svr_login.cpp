#include "stdafx.h"
#include "test_generated.h"

using namespace TestApp;

int main(int argc, char* argv[])
{
    flatbuffers::FlatBufferBuilder builder;

    std::vector<uint64> vec;
    for (size_t i = 0; i < 10; i++)
    {
        vec.push_back(i);
    }
    // Create flat buffer inner type
    auto id = 123;
    auto name = builder.CreateString("name");
    auto list = builder.CreateVector(vec); // vector
    auto flag = 233;
    auto kv = KV(1, 1.0); // struct
    // table
    auto mloc = CreateTestObj(builder, id, name, flag, list, &kv);
    builder.Finish(mloc);

    char* ptr = (char*)builder.GetBufferPointer();
    uint32 size = builder.GetSize();

    ////////// Deserialize //////////
    auto obj = GetTestObj(ptr);

    cout << obj->id() << endl;
    cout << obj->name()->c_str() << endl;
    cout << int(obj->flag()) << endl;
    for (size_t i = 0; i < obj->list()->size(); i++)
    {
        cout << obj->list()->Get(i) << endl;
    }
    cout << obj->kv()->key() << endl;
    cout << obj->kv()->value() << endl;


	system("pause");
	return 0;
}