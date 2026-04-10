-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local test_base64, test_sha256, test_hmac, test_uuid, test_password

local crypto = require("src/stdlib/crypto")

function test_base64()
    print("--- Testando Base64 ---")
    local original = "Zenith"
    local encoded = crypto.base64_encode(original)
    print(zt.add("Original: ", original))
    print(zt.add("Encoded:  ", encoded))
    local decoded_res = crypto.base64_decode(encoded)
    local decoded = decoded_res:unwrap_or("erro")
    print(zt.add("Decoded:  ", decoded))
    if (original == decoded) then
        print("✅ Base64 OK!")
    end
end

function test_sha256()
    print("\
--- Testando SHA-256 ---")
    local hash = crypto.sha256("zenith")
    print(zt.add("Hash de 'zenith': ", hash))
    if hash:starts_with("0d51704") then
        print("✅ SHA-256 OK!")
    else
        print("❌ SHA-256 incorreto")
    end
end

function test_hmac()
    print("\
--- Testando HMAC-SHA256 ---")
    local hmac = crypto.hmac_sha256("key", "data")
    print(zt.add("HMAC: ", hmac))
    if hmac:starts_with("5031fe") then
        print("✅ HMAC OK!")
    else
        print("❌ HMAC incorreto")
    end
end

function test_uuid()
    print("\
--- Testando UUID v4 ---")
    local id1 = crypto.generate_uuid()
    local id2 = crypto.generate_uuid()
    print(zt.add("UUID 1: ", id1))
    print(zt.add("UUID 2: ", id2))
    if (id1 ~= id2) then
        print("✅ UUIDs são diferentes!")
    end
    if (#(id1) == 36) then
        print("✅ Formato do UUID OK!")
    end
end

function test_password()
    print("\
--- Testando Password Helper ---")
    local p1 = crypto.hash_password("123456", "salt")
    local p2 = crypto.hash_password("123456", "salt")
    local p3 = crypto.hash_password("123456", "other_salt")
    if ((p1 == p2) and (p1 ~= p3)) then
        print("✅ Password hashing com salt OK!")
    end
end

-- Struct Methods
test_base64()

test_sha256()

test_hmac()

test_uuid()

test_password()


return {
}