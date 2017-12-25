require("check_session");

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

--init request handler
if ngx.var.message_type == "1" then
    if ngx.var.message_data == "ok" then
        local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
        if res.status == 200 then
            ngx.var.init_mysql_query_result = res.body;
            if ngx.var.init_mysql_query_status == "empty" then
                --插入到用户表中
                local res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
                if res.status == 200 then
                    ngx.var.init_mysql_insert_result = res.body;
                    --uid<->session写入缓存
                    local res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
                    --插入用户成就数据到表中
                    local res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
                else
                    ngx.var.init_mysql_insert_result = "error";
                end
            else
                local res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
            end
        else
            ngx.var.init_mysql_query_result = "error";
        end
    end
    ngx.print(ngx.var.init_resp);
    goto log_content;
end

--register handler
if ngx.var.message_type == "2" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            local res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.register_mysql_result = res.body;
            else
                ngx.var.register_mysql_result = "error";
            end
        end
    end
    ngx.print(ngx.var.register_resp);
    goto log_content;
end

--login handler
if ngx.var.message_type == "3" then
    if ngx.var.message_data == "ok" then
        local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
        if res.status == 200 then
            ngx.var.login_mysql_query_result = res.body;
            if ngx.var.login_mysql_query_status == "ok" then
                local res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
                res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
            end
        else
            ngx.var.login_mysql_query_result = "error";
        end
    end
    ngx.print(ngx.var.login_resp);
    goto log_content;
end

--set user profile handler
if ngx.var.message_type == "4" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            local res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.set_user_prof_mysql_result = res.body;
            else
                ngx.var.set_user_prof_mysql_result = "error";
            end
        end
    end
    ngx.print(ngx.var.set_user_prof_resp);
    goto log_content;
end

--get user profile handler
if ngx.var.message_type == "5" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.get_user_prof_mysql_result = res.body;
            else
                ngx.var.get_user_prof_mysql_result = "error";
            end
        end
    end
    ngx.print(ngx.var.get_user_prof_resp);
    goto log_content;
end

--get user achievement handler
if ngx.var.message_type == "6" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            if ngx.var.cache_achievement ~= nil then
            end
            local res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
            if res.status == 200 then
                ngx.var.get_user_achieve_result_r = res.body;
            else
                ngx.var.get_user_achieve_result_r = "error";
            end
            if ngx.var.get_user_achieve_result_status_r ~= "ok" then
                local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
                if res.status == 200 then
                    ngx.var.get_user_achieve_result_m = res.body;
                    local res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
                else
                    ngx.var.get_user_achieve_result_m = "error";
                end
            end
        end
    end
    ngx.print(ngx.var.get_user_achieve_resp);
    goto log_content;
end

--get user ugc list handler
if ngx.var.message_type == "7" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.get_user_ugc_list_result = res.body;
            else
                ngx.var.get_user_ugc_list_result = "error";
            end
        end
    end
    ngx.print(ngx.var.get_user_ugc_list_resp);
    goto log_content;
end

--check user register handler
if ngx.var.message_type == "8" then
    if ngx.var.message_data == "ok" then
        local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
        if res.status == 200 then
            ngx.var.check_user_reg_result = res.body;
        else
            ngx.var.check_user_reg_result = "error";
        end
    end
    ngx.print(ngx.var.check_user_reg_resp);
    goto log_content;
else
    return ngx.exit(ngx.HTTP_NOT_FOUND);
end

::log_content::
if ngx.var.pvlog == nil then
end

return ngx.exit(ngx.OK);
