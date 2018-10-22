local this = Module.test1

local test2 = Module.test2

function this.Init()
    print("---1 Init---")
end
function this.OnStart()
    print("---1 OnStart---")
    this.add(1,2)
end

this.str = "I am so cool"
this.tbl = {name = "1胡椒", id = 10114442}
this.tbl.t = { id = "233", str = "zhoumf", id2 = "zhoumf" }
this.tbl.t.t = this.tbl
function this.add(a,b)
    print(debug.traceback())
    print("---1aaaaa---")
    print(test2.tbl.name)
    return a + b
end
