// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xloader/src/xgenesis_info.h"

namespace top {

static std::string const g_ci_genesis_config =
R"T(
{
    "accounts": {
        "tcc": {
            "T00000LcxcHVTKki5KqCKmX5BbbMSGrUPhTEpwnu": {"balance": "1000000000"},
            "T00000LfazE9WjtUu4xx5caD9w9dWwiRGggHxxvo": {"balance": "1000000000"},
            "T00000LfyErVp716mVR89UdJMQaZJ6W9Hv7NsHT4": {"balance": "1000000000"}
        },
        "genesis_funds_account": {
            "T00000Lhj29VReFAT958ZqFWZ2ZdMLot2PS5D5YC": {"balance": "2999997000000000"},
            "T00000LhPZXie5GqcZqoxu6BkfMUqo7e9x1EaFS6": {"balance": "6000000000000000"},
            "T00000LKMWBfiHPeN9TShPdB9ctLDr7mmcZa46Da": {"balance": "3400000000000000"}
        }
    },
    "seedNodes": {
        "T00000LKy8CAPRETJmUr9DZiwCPcc9VFqDBpQEG6": "BO1GzXnnDufgfv/OYyBGA4b/cgq87H3F0tCkeQiR5b4mePf8V721WoXPHGgvWPdQ2Ci0SsL4VSrSLVdKeheNdyk=",
        "T00000LLLK8teSrpbtcVnaYCtykniJHHeMZegQMU": "BHWT6Jr004YIsfnhtUkLl2ZK7Z6gxfhaktgxib+8dKlOhqkTxib0bMQKtrjuKHQWVE2DaEF7QQiMWUeF9mEt59E=",
        "T00000LPjk6KqsnCu46kHffttVPjToRmitpStvvo": "BKnMfrDLOw2tyuCBK7Fqa+pHp4o8/fQKLVN8EPGxkzEy2wz6rjR2TrKasn2m0R7yEEa1DM3iCjOwyAxiq3eAdcQ=",
        "T00000LQkSo2PYR7hVxZmkQphvsxAbiB8V22PwPT": "BJE+S/NmkfMB8OIzHoJcy+lrBs0yD7X0dbKmYTg4PaNzLpaPIG9x5ojAJjSDm4JzpE84YN2C09nenWtJo0Qqxx4=",
        "T00000LRauRZ3SvMtNhxcvoHtP8JK9pmZgxQCe7Q": "BBZQTi7Pv4yRCYzFxri1BRUWXV6hd3nXG1tV21niKRfhk9sPmqX5SA1PcPhe3ZECIILpuvceiguSh6arX6Wfd0k=",
        "T00000LRc5LxEXVdUuFcoLezjPiMw4CLYgvkEmtC": "BAR31Ok8x+EXZIMJNhQhgHXdvEfbJJ6EajJn0OZf0jPC3NKzyAAmTP86LPwATfeE+KyZUFom5tdCErx4ThlgQ4Q="
    },
    "timestamp": 1599555555
}
)T";

const std::string & get_genesis_info() {
    return g_ci_genesis_config;
}

}
