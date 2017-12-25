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
--upload ugc handler
if ngx.var.message_type == "101" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            --插入消息表
            local res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.upload_mysql_result = res.body;
                if ngx.var.upload_mysql_result_status ~= "ok" then
                    goto upload_resp;
                end
            else
                ngx.var.upload_mysql_result = "error";
                goto upload_resp;
            end
            --插入消息成就表
            res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.insert_message_achieve_result = res.body;
                if ngx.var.insert_message_achieve_result_status ~= "ok" then
                    goto upload_resp;
                end
            else
                ngx.var.insert_message_achieve_result = "error";
                goto upload_resp;
            end
            --插入消息种子表
            res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.insert_message_seed_result = res.body;
                if ngx.var.insert_message_seed_result_status ~= "ok" then
                    goto upload_resp;
                end
            else
                ngx.var.insert_message_seed_result = "error";
                goto upload_resp;
            end
            --更新用户成就数据，先从缓存获取
            res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
            if res.status == 200 then
                ngx.var.upload_get_user_achieve_result = res.body;
                --获取成功，更新缓存，并更新数据库
                if ngx.var.upload_get_user_achieve_result_status == "ok" then
                    local res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
                end 
            else
                ngx.var.upload_get_user_achieve_result = "error";
            end
            --用户成就数据写入数据库
            res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
        end
    end
::upload_resp::
    ngx.print(ngx.var.upload_resp);
    goto log_content;
end

--pull ugcc list handler
if ngx.var.message_type == "102" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            local step = 1;
            math.randomseed(os.time());
            local seed = math.random(100);
            --40% rate to select ugc by local cell
            if 1 <= seed and seed <= 40 then
                ngx.var.pull_ugc_list_type = "local";
                local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
                if res.status == 200 then
                    ngx.var.pull_ugc_list_mysql_result = res.body;
                    if ngx.var.pull_ugc_list_has_local_ugc == "ok" then
                        goto pull_resp;
                    else
                        step = 2;
                    end
                else
                    ngx.var.pull_ugc_list_mysql_result = "error";
                end
            end
            --if select local failed or 40% rate by neighbor
            if step == 2 or (41 <= seed and seed <= 80) then
                ngx.var.pull_ugc_list_type = "neighbor";
                local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
                if res.status == 200 then
                    ngx.var.pull_ugc_list_mysql_result = res.body;
                    if ngx.var.pull_ugc_list_has_neighbor_ugc == "ok" then
                        goto pull_resp;
                    else
                        step = 3;
                    end
                else
                    ngx.var.pull_ugc_list_mysql_result = "error";
                end
            end
            --select ugc by far
            local pull_type = ngx.var.pull_ugc_list_pull_type;
            if pull_type == "pull" then
                ngx.var.pull_ugc_list_type = "far";
                local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
                if res.status == 200 then
                    ngx.var.pull_ugc_list_mysql_result = res.body;
                    if ngx.var.pull_ugc_list_has_far_ugc == "ok" then
                        goto pull_resp;
                    end
                else
                    ngx.var.pull_ugc_list_mysql_result = "error";
                end
            end
        end
    end
::pull_resp::
    ngx.print(ngx.var.pull_ugc_list_resp);
    goto log_content;
end

--get ugc detail handler
if ngx.var.message_type == "103" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.get_ugc_mysql_result = res.body;
            else
                ngx.var.get_ugc_mysql_result = "error";
            end
        end
    end
    ngx.print(ngx.var.get_ugc_resp);
    goto log_content;
end

--post ugc msg handler
if ngx.var.message_type == "104" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            --写入post表或者pass表
            local res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.post_ugc_mysql_result = res.body;
                if ngx.var.post_ugc_mysql_result_status ~= "ok" then
                    goto post_resp;
                end
            else
                ngx.var.post_ugc_mysql_result = "error";
                goto post_resp;
            end
            --pass加入屏蔽列表
            if ngx.var.post_ugc_type == "post" then
                --如果是增值操作，那么就把结果写入redis，用来过滤；目前不考虑
            else
                --如果是消灭，那么把该条消息加入到黑名单中，用来过滤
                res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
            end
            --获取消息成就数据
            local ugc_achieve_types = {"msg", "seed"};
            for i = 1, 2 do
                ngx.var.post_ugc_select_achieve_type = ugc_achieve_types[i];
                --先获取该条消息的成就数据信息; msg表示获取原始消息表成就数据；seed表示从网格种子表获取
                res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
                if res.status == 200 then
                    ngx.var.post_ugc_select_achieve_result = res.body;
                    if i == 1 then
                        if ngx.var.post_ugc_select_msg_achieve_result_status ~= "ok" then
                            goto post_resp;
                        end
                    else
                        if ngx.var.post_ugc_selects_seed_achieve_result_status ~= "ok" then
                            goto post_resp;
                        end
                    end
                else
                    ngx.var.post_ugc_select_achieve_result = "error";
                    goto post_resp;
                end
            end
            --更新消息成就数据
            for i = 1, 2 do
                ngx.var.post_ugc_update_achieve_type = ugc_achieve_types[i];
                res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
                if res.status == 200 then
                    ngx.var.post_ugc_update_achieve_result = res.body;
                else
                    ngx.var.post_ugc_update_achieve_result = "error";
                end
                --如果消息不是来自自身网格，并且是增值操作，那么需要插入种子到新网格
                if i == 2 and ngx.var.post_ugc_seed_is_local ~= "ok" then
                    res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
                end
            end
            --更新用户成就数据
            local user_achieve_types = {"from", "to"};
            for i = 1, 2 do
                ngx.var.post_ugc_user_achieve_type = user_achieve_types[i];
                --更新用户成就数据，先从缓存获取; from表示更新消息来源者成就；to更新自己成就
                res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
                if res.status == 200 then
                    ngx.var.post_ugc_user_achieve_result = res.body;
                    --获取成功，更新缓存，并更新数据库
                    if i == 0 then
                        if ngx.var.post_ugc_user_from_achieve_result_status == "ok" then
                            local res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
                        end
                    else
                        if ngx.var.post_ugc_user_to_achieve_result_status == "ok" then
                            local res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
                        end
                    end
                else
                    ngx.var.post_ugc_user_achieve_result = "error";
                end
                --用户成就数据写入数据库
                res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
            end
        end
    end
::post_resp::
    ngx.print(ngx.var.post_ugc_resp);
    goto log_content;
end

--add ugc comment handler
if ngx.var.message_type == "105" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            local res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.add_comment_mysql_result = res.body;
            else
                ngx.var.add_comment_mysql_result = "error";
            end
        end
    end
    ngx.print(ngx.var.add_comment_resp);
    goto log_content;
end

--get comment handler
if ngx.var.message_type == "106" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.get_comment_mysql_result = res.body;
            else
                ngx.var.get_comment_mysql_result = "error";
            end
        end
    end
    ngx.print(ngx.var.get_comment_resp);
    goto log_content;
end

--add collection msg
if ngx.var.message_type == "107" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            local res = ngx.location.capture("/mysql/read?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.query_clt_mysql_result = res.body;
                if ngx.var.add_collection_status == "ok" then
                    local res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
                    if res.status == 200 then
                        ngx.var.add_clt_mysql_result = res.body;
                    else
                        ngx.var.add_clt_mysql_result = "error";
                    end
                end
            end
        end
    end
    ngx.print(ngx.var.add_collection_resp);
    goto log_content;
end

--add pk msg
if ngx.var.message_type == "108" then
    if ngx.var.message_data == "ok" then
        if check_session() ~= false then
            --更新用户成就数据，先从缓存获取
            res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
            if res.status == 200 then
                ngx.var.pk_get_user_achieve_result = res.body;
                --获取成功，更新缓存，并更新数据库
                if ngx.var.pk_get_user_achieve_result_status == "ok" then
                    local res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
                end 
            else
                ngx.var.pk_get_user_achieve_result = "error";
            end
            local res = ngx.location.capture("/mysql/write?" .. ngx.var.mysql_query_string);
            if res.status == 200 then
                ngx.var.pk_mysql_result = res.body;
            else
                ngx.var.pk_mysql_result = "error";
            end
        end
    end
    ngx.print(ngx.var.pk_resp);
    goto log_content;
else
    return ngx.exit(ngx.HTTP_NOT_FOUND);
end

::log_content::
if ngx.var.pvlog == nil then
end

return ngx.exit(ngx.OK);
