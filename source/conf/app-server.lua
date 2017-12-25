--使用随机搜索附近网格的概率，因为前期数据少，该策略效果可能不好所以
--这时候，要使用周围9个网格搜索的策略；返回1，只使用附近一个网格搜索，
--否则，在附近9个网格搜索

function random_neighbor_rate(rate)
    if rate > 0 then
        local i = math.random(100);
        if i <= rate then
            return 1;
        end
        return 0;
    end
    return 0;
end

math.randomseed(os.time());
