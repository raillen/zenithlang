return {
    names = {
        "zt.selfhost.has_slot",
        "zt.selfhost.text_len",
        "zt.selfhost.text_slice",
        "zt.selfhost.text_replace",
        "zt.selfhost.value_is_present",
        "zt.selfhost.empty_value",
        "zt.selfhost.read_module_source",
        "zt.selfhost.fold_number_binary",
        "zt.selfhost.host_os",
        "zt.selfhost.compile_result_text",
        "zt.selfhost.run_cli",
    },
    corpus = {
        "lua ztc.lua --strict-selfhost --version",
        "lua ztc.lua --strict-selfhost check src/compiler/syntax.zt",
        "lua ztc.lua --strict-selfhost build demo.zt .selfhost-artifacts/release/selfhost-release-demo.lua",
        "lua ztc.lua --strict-selfhost run tests/stdlib/test_optional.zt",
        "lua ztc.lua --strict-selfhost zpm help",
        "lua ztc.lua --strict-selfhost zpm doctor",
        "lua ztc.lua --strict-selfhost zman list",
        "lua ztc.lua --strict-selfhost zman show std.core",
        "lua ztc.lua --strict-selfhost ztest --help",
        "lua ztc.lua --strict-selfhost ztest --grep parser_tests/test_parser.lua",
    }
}
