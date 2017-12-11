
local this          = test_LogicMail
local AssertEqual   = Test.AssertEqual
local NewBuffer     = Test.NewBuffer
local Token         = Token


function this.ClearMail(user)
    for _,v in pairs(user.mail()) do
        user.mail[v.id] = nil
    end
end

-- 发邮件
function this.test_SendMail(user)
    this.ClearMail(user)
    local content = {"单元测试", "渣渣", "啊啊啊啊啊啊啊"}
    LogicMail.SendMail(user, content)
    for _,v in pairs(user.mail()) do
        AssertEqual(v.content, content)
    end
end

-- 全服邮件：在线直接收到，离线的登录时收到
function this.test_SendSvrMail(user)
    this.ClearMail(user)
    local content = {"单元测试", "渣渣", "啊啊啊啊啊啊啊"}
    LogicMail.SendSvrMail(content)
    for _,v in pairs(user.mail()) do
        -- AssertEqual(v.content, content) -- 异步发，不会及时收到
    end
end

-- 读全部邮件
function this.test_1(user)
    this.ClearMail(user)
    local content = {"单元测试", "渣渣", "啊啊啊啊啊啊啊"}
    local attachment = {money = 100, gold = 2000}
    local oldMoney, oldGold = Token.get(user, 'money'), Token.get(user, 'gold')
    LogicMail.SendMail(user, content, attachment)

	local buf = NewBuffer()
    buf:BeginWrite()
	buf:WriteUInt8(0)
    buf:BeginRead()

	LogicMail.rpc_mail_read(user, buf)
    for _,v in pairs(user.mail()) do
        AssertEqual(v.isread, 1)
        local newMoney, newGold = Token.get(user, 'money'), Token.get(user, 'gold')
        AssertEqual(newMoney, oldMoney+100)
        AssertEqual(newGold, oldGold+2000)
    end
end

-- 读一封
function this.test_2(user)
    this.ClearMail(user)
    local content = {"单元测试", "渣渣", "啊啊啊啊啊啊啊"}
    local attachment = {money = 100, gold = 2000}
    local oldMoney, oldGold = Token.get(user, 'money'), Token.get(user, 'gold')
    LogicMail.SendMail(user, content, attachment)

    local id = nil
    for _,v in pairs(user.mail()) do
        id = v.id
        break
    end

    local buf = NewBuffer()
    buf:BeginWrite()
    buf:WriteUInt8(1)
    buf:WriteUInt64(id)
    buf:BeginRead()

    LogicMail.rpc_mail_read(user, buf)
    AssertEqual(user.mail[id], nil)

    local newMoney, newGold = Token.get(user, 'money'), Token.get(user, 'gold')
    AssertEqual(newMoney, oldMoney+100)
    AssertEqual(newGold, oldGold+2000)
end

-- 读已读的
function this.test_3(user)
    this.ClearMail(user)
    local content = {"单元测试", "渣渣", "啊啊啊啊啊啊啊"}
    local attachment = {money = 100, gold = 2000}
    LogicMail.SendMail(user, content, attachment)

    local id = nil
    for _,v in pairs(user.mail()) do
        id = v.id
        break
    end

    local buf = NewBuffer()
    buf:BeginWrite()
    buf:WriteUInt8(1)
    buf:WriteUInt64(id)
    buf:WriteUInt8(1)   -- 读两次，所以要写两份数据
    buf:WriteUInt64(id)
    buf:BeginRead()

    -- 第一次读，有效
    LogicMail.rpc_mail_read(user, buf)
    local oldMoney, oldGold = Token.get(user, 'money'), Token.get(user, 'gold')

    -- 第二次读，无效
    AssertEqual(LogicMail.rpc_mail_read(user, buf), -1)
    local newMoney, newGold = Token.get(user, 'money'), Token.get(user, 'gold')
    AssertEqual(newMoney, oldMoney)
    AssertEqual(newGold, oldGold)
end
