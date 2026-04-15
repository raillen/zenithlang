local TEXT = require("src/stdlib/text")

local VALIDATION = {}

local function measure(value)
    if type(value) == "string" then
        return TEXT.count_chars(value)
    end
    if type(value) == "table" then
        return #value
    end
    return 0
end

function VALIDATION.min_value(it, min)
    return type(it) == "number" and type(min) == "number" and it >= min
end

function VALIDATION.max_value(it, max)
    return type(it) == "number" and type(max) == "number" and it <= max
end

function VALIDATION.between(it, min, max)
    return VALIDATION.min_value(it, min) and VALIDATION.max_value(it, max)
end

function VALIDATION.one_of(it, options)
    if type(options) ~= "table" then
        return false
    end

    for _, option in ipairs(options) do
        if option == it then
            return true
        end
    end

    return false
end

function VALIDATION.non_empty(it)
    if type(it) == "string" then
        return it ~= ""
    end
    if type(it) == "table" then
        return #it > 0
    end
    return false
end

function VALIDATION.not_blank(it)
    return type(it) == "string" and TEXT.trim(it) ~= ""
end

function VALIDATION.len_min(it, min)
    return measure(it) >= min
end

function VALIDATION.len_max(it, max)
    return measure(it) <= max
end

function VALIDATION.len_between(it, min, max)
    local size = measure(it)
    return size >= min and size <= max
end

function VALIDATION.matches(it, pattern)
    return type(it) == "string" and type(pattern) == "string" and it:match(pattern) ~= nil
end

function VALIDATION.email(it)
    return type(it) == "string" and it:match("^[^%s@]+@[^%s@]+%.[^%s@]+$") ~= nil
end

function VALIDATION.url(it)
    return type(it) == "string" and it:match("^https?://[%w%-._~:/%?#%[%]@!$&'()*+,;=]+$") ~= nil
end

function VALIDATION.uuid(it)
    return type(it) == "string"
        and it:match("^%x%x%x%x%x%x%x%x%-%x%x%x%x%-%x%x%x%x%-%x%x%x%x%-%x%x%x%x%x%x%x%x%x%x%x%x$") ~= nil
end

return VALIDATION
