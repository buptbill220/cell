local parser = require("redis.parser")
local json = require("cjson")

local res = ngx.location.capture("/redis-raw?" .. ngx.var.query_string .. "%0d%0a")

if res.status == 200 then
    local ret, typ = parser.parse_reply(res.body)
    ngx.print(ret)
    ngx.exit(ngx.OK) --200
else
    ngx.exit(204) --HTTP_NO_CONTENT
end
