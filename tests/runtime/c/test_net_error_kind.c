#include "runtime/c/zenith_rt.h"

#include <stdio.h>
#include <stdlib.h>

static void assert_true(const char *name, int condition) {
    if (!condition) {
        fprintf(stderr, "falha no teste %s\n", name);
        exit(1);
    }
}

static void assert_kind_index(const char *name, const char *code, int expected) {
    zt_core_error error = zt_core_error_from_message(code, "network failure");
    int actual = (int)zt_net_error_kind_index(error);

    assert_true(name, actual == expected);
    zt_core_error_dispose(&error);
}

static void test_known_kind_indexes(void) {
    assert_kind_index("net_connection_refused", "net.ConnectionRefused", 1);
    assert_kind_index("net_host_unreachable", "net.HostUnreachable", 2);
    assert_kind_index("net_dns_failed_alias", "net.DnsFailed", 2);
    assert_kind_index("net_timeout", "net.Timeout", 3);
    assert_kind_index("net_address_in_use", "net.AddressInUse", 4);
    assert_kind_index("net_already_connected", "net.AlreadyConnected", 5);
    assert_kind_index("net_not_connected", "net.NotConnected", 6);
    assert_kind_index("net_network_down", "net.NetworkDown", 7);
    assert_kind_index("net_overflow", "net.Overflow", 8);
    assert_kind_index("net_peer_reset", "net.PeerReset", 9);
    assert_kind_index("net_system_limit", "net.SystemLimit", 10);
}

static void test_unknown_and_missing_codes(void) {
    zt_core_error unknown = zt_core_error_from_message("net.SomethingElse", "unknown network failure");
    zt_core_error missing = {0};

    assert_true("net_unknown_code", zt_net_error_kind_index(unknown) == 0);
    assert_true("net_missing_code", zt_net_error_kind_index(missing) == 0);

    zt_core_error_dispose(&unknown);
}

static void test_repeated_classification(void) {
    zt_core_error error = zt_core_error_from_message("net.Timeout", "loop");
    int index = 0;
    int i;

    for (i = 0; i < 200000; i += 1) {
        index = (int)zt_net_error_kind_index(error);
        if (index != 3) {
            break;
        }
    }

    assert_true("net_repeated_classification", index == 3);
    zt_core_error_dispose(&error);
}

int main(void) {
    test_known_kind_indexes();
    test_unknown_and_missing_codes();
    test_repeated_classification();
    puts("Runtime net error tests OK");
    return 0;
}
