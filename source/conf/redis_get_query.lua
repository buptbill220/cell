local parser = require("redis.parser")

local res = ngx.location.capture("/redis-get?" .. ngx.var.query_string)

if res.status == 200 then
    local ret, typ = parser.parse_reply(res.body)
    ngx.print(ret)
    ngx.exit(ngx.OK) --200
else
    ngx.exit(204) --HTTP_NO_CONTENT
end
