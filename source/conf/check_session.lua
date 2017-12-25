function check_session()
    local res = ngx.location.capture("/redis/raw?" .. ngx.var.redis_query_string);
    if res.status == 200 then
        ngx.var.user_session = res.body;
        if ngx.var.check_session == "ok" then
            return true;
        end
    end
    return false;
end
