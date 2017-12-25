function check_key(args, body_length)
    if args ~= nil then
        local check = math.floor((body_length + 99999989) / 3);
        return (tonumber(args) == check);
    end
    return false;
end

--校验数据
if ngx.var.request_method ~= "POST" then
    return ngx.exit(ngx.HTTP_NOT_ALLOWED);
end

content_type = ngx.var.http_content_type;
--pos = string.find(content_type, ";", 1);
--if pos ~= nil then
--    content_type = string.sub(content_type, 1, pos -1);
--end

if content_type ~= "application/x-protobuf" then
    return ngx.exit(ngx.HTTP_BAD_REQUEST);
end

ngx.req.read_body();
 
local check = check_key(ngx.var.arg_k, string.len(ngx.var.request_body));

if check == false then
    return ngx.exit(ngx.HTTP_BAD_REQUEST);
end

ngx.var.message = ngx.var.request_body;

--设置返回头
ngx.header.content_type = "application/x-protobuf";
