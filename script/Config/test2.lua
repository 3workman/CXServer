local this = Module.test2

local test1 = Module.test1

function this.OnAwake()
    print("---2 OnAwake---")
end

this.str = "2zhoumf 233!"
this.tbl = {name = "2zhoumf", id = 20114442}
function this.add(a,b)
    print("---2bbbbb---")
    print(test1.tbl.name)
    return a + b
end