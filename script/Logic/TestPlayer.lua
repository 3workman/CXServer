print("--- TestPlayer ---")

local this = Module.TestPlayer

this.str = "I am so cool"
this.tbl = {name = "shun", id = 20114442}

InitThisValue(this,"a",0)
function this.upvalue1()
    this.a = this.a + 1
    print("upvalue------------"..this.a)
end
local a = 0
function this.upvalue2()
    a = a + 1
    print("upvalue------------"..a)
end

this.upvalue1()
this.upvalue2()