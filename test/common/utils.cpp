// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/common/utils.hpp"

TEST_CASE("We are NOT using the default random seed") {
    // Note: the default random generator shall be seeded using 1 unless
    // seeded otherwise, according to Linux and MacOS manpages
    REQUIRE(mk::random_str(16) != "2PN0bdwPY7CA8M06");
}

TEST_CASE("random_within_charset() works with zero length string") {
    REQUIRE_THROWS_AS(mk::random_within_charset("", 16),
                      const std::runtime_error &);
}

TEST_CASE("random_within_charset() uses all the available charset") {
    REQUIRE(mk::random_within_charset("x", 4) == "xxxx");
}

TEST_CASE("random_printable() generates printable characters") {
    for (auto x : mk::random_str(65536)) {
        REQUIRE((x >= ' ' && x <= '~'));
    }
}

TEST_CASE("random_str() really generates only characters or numbers") {
    auto found_num = false;
    auto found_upper = false;
    auto found_low = false;
    for (auto x : mk::random_str(1024)) {
        if (isdigit(x)) {
            found_num = true;
        } else if (isupper(x)) {
            found_upper = true;
        } else if (islower(x)) {
            found_low = true;
        } else {
            REQUIRE(false); /* Should not happen */
        }
    }
    REQUIRE(found_num);
    REQUIRE(found_upper);
    REQUIRE(found_low);
}

TEST_CASE("random_str_uppercase() really generates only uppercase") {
    auto found_num = false;
    auto found_upper = false;
    for (auto x : mk::random_str_uppercase(1024)) {
        if (isdigit(x)) {
            found_num = true;
        } else if (isupper(x)) {
            found_upper = true;
        } else {
            REQUIRE(false); /* Should not happen */
        }
    }
    REQUIRE(found_num);
    REQUIRE(found_upper);
}

TEST_CASE("random_str_lower_alpha() does what it should") {
    std::string lower_alpha = "abcdefghijklmnopqrstuvwxyz"; // works in my locale
    for (auto x : mk::random_str_lower_alpha(1024)) {
        REQUIRE(lower_alpha.find(x) != std::string::npos);
    }
}

TEST_CASE("random_tld() does what it should") {
    std::vector<std::string> tlds =
        { ".com", ".net", ".org", ".info", ".test", ".invalid" };
    for (size_t i = 0; i < 100; i++) {
        auto x = mk::random_tld();
        REQUIRE(std::find(tlds.begin(), tlds.end(), x) != tlds.end());
    }
}

TEST_CASE("split(std::string s) works properly in the common case") {
    REQUIRE((mk::split(" 34    43  17 11 ") == std::list<std::string>{
                {"", "34", "43", "17", "11"}
            }));
}

TEST_CASE("split(std::string s) works properly with only one token") {
    REQUIRE((mk::split("34") == std::list<std::string>{
                {"34"}
            }));
}

TEST_CASE("median() works as expected with zero length vector") {
    REQUIRE_THROWS(mk::median({}));
}

TEST_CASE("median() works as expected for vectors with odd elements") {
    // Note: data computed using python3's median
    std::vector<double> v{
        0.01596747804328924,  0.2450830526507135,     0.9353536700351008,
        0.7863802913414082,   0.00045868623508005246, 0.9277142006103589,
        0.08697013493193806,  0.5903717833543904,     0.4408202499601306,
        0.4110671418635252,   0.24814465175517897,    0.6139834690241912,
        0.9608206509528763,   0.2540680139459752,     0.11506166524419636,
        0.024724256379755394, 0.896471120293437};
    REQUIRE((v.size() % 2) != 0);
    REQUIRE(mk::median(v) == Approx(0.4110671418635252));
}

TEST_CASE("median() works as expected for vectors with even elements") {
    // Note: data computed using python3's median
    std::vector<double> v{
        0.9230726199832225,  0.8522186171268678,  0.12185510369876285,
        0.03466879744757101, 0.24972504270002582, 0.08396440655815418,
        0.9331588554475844,  0.6661070951871151,  0.020188724237295563,
        0.230362677725527,   0.6949929576413474,  0.3068761654317057,
        0.7084977916078418,  0.08247376181844912, 0.5230767427503158,
        0.0469593072933856,
    };
    REQUIRE((v.size() % 2) == 0);
    REQUIRE(mk::median(v) == Approx(0.27830060406586576));
}

TEST_CASE("percentile() works as expected with zero length vector") {
    REQUIRE_THROWS(mk::percentile({}, 0.1));
}

TEST_CASE("percentile() works as expected for vectors with odd elements") {
    // Note: data computed using a Google Doc spreadsheet
    std::vector<double> v{
        0.2227235716, 0.4668712435, 0.2772333249, 0.7749811255,   0.2253996621,
        0.6331470207, 0.8326146628, 0.5691038594, 0.2250193985,   0.9313505106,
        0.644014243,  0.7353389179, 0.5380223015, 0.007788676186, 0.6278004028,
        0.8542452236, 0.3845593954, 0.3822451374, 0.1697755851,
    };
    REQUIRE((v.size() % 2) != 0);
    REQUIRE(mk::percentile(v, 0.1) == Approx(0.2121339743));
    REQUIRE(mk::percentile(v, 0.9) == Approx(0.836940775));
}

TEST_CASE("percentile() works as expected for vectors with even elements") {
    // Note: data computed using a Google Doc spreadsheet
    std::vector<double> v{
        0.3663851332, 0.008667705304, 0.5253419622,  0.7734632011,
        0.2787519528, 0.8274162592,   0.2362169824,  0.3801579128,
        0.7136612724, 0.6310992347,   0.03243008666, 0.03001627822,
        0.9996497564, 0.6921566966,   0.03240005956, 0.1672297861,
        0.7255344458, 0.6237373722,   0.1458365347,  0.3131758033,
    };
    REQUIRE((v.size() % 2) == 0);
    REQUIRE(mk::percentile(v, 0.1) == Approx(0.03216168143));
    REQUIRE(mk::percentile(v, 0.9) == Approx(0.7788585069));
}

TEST_CASE("percentile() works with one-element vectors") {
    REQUIRE(mk::percentile({17.0}, 0.5) == 17.0);
    REQUIRE(mk::percentile({17.0}, 0.1) == 17.0);
    REQUIRE(mk::percentile({17.0}, 0.9) == 17.0);
}

TEST_CASE("sha256_of() works as expected") {
    REQUIRE(mk::sha256_of("xeuCh5zu chai5oeL uv0foh4E Ixiew5Uc thaid6Vu") ==
            "7a8f31f91ddabd2ee96230b512b27f5a88adeceb20cc08228819b77417fba96e");
}

static FILE *fopen_fail(const char *, const char *) {
    return nullptr;
}

static int fseek_fail(FILE *, long, int) {
    return -1;
}

static long ftell_fail(FILE *) {
    return -1;
}

static size_t fread_fail(void *, size_t, size_t, FILE *) { return 0; }

static int fclose_fail(FILE *) { return -1; }

TEST_CASE("slurpv() works as expected") {
    SECTION("If fopen() fails") {
        auto maybe_res = mk::slurpv<char, fopen_fail>(
            "./test/fixtures/text-with-utf8.txt");
        REQUIRE(!maybe_res);
        REQUIRE(maybe_res.as_error() == mk::FileIoError());
    }

    SECTION("If first fseek() fails") {
        auto maybe_res = mk::slurpv<char, std::fopen, fseek_fail>(
            "./test/fixtures/text-with-utf8.txt");
        REQUIRE(!maybe_res);
        REQUIRE(maybe_res.as_error() == mk::FileIoError());
    }

    SECTION("If ftell() fails") {
        auto maybe_res =
            mk::slurpv<char, std::fopen, std::fseek, ftell_fail>(
                "./test/fixtures/text-with-utf8.txt");
        REQUIRE(!maybe_res);
        REQUIRE(maybe_res.as_error() == mk::FileIoError());
    }

    SECTION("If second fseek() fails") {
        auto maybe_res =
            mk::slurpv<char, std::fopen, std::fseek, std::ftell,
                            fseek_fail>("./test/fixtures/text-with-utf8.txt");
        REQUIRE(!maybe_res);
        REQUIRE(maybe_res.as_error() == mk::FileIoError());
    }

    SECTION("If fread() fails") {
        auto maybe_res = mk::slurpv<char, std::fopen, std::fseek,
                                         std::ftell, std::fseek, fread_fail>(
            "./test/fixtures/text-with-utf8.txt");
        REQUIRE(!maybe_res);
        REQUIRE(maybe_res.as_error() == mk::FileIoError());
    }

    SECTION("If fclose() fails") {
        auto maybe_res =
            mk::slurpv<char, std::fopen, std::fseek, std::ftell,
                            std::fseek, std::fread, fclose_fail>(
                "./test/fixtures/text-with-utf8.txt");
        REQUIRE(!maybe_res);
        REQUIRE(maybe_res.as_error() == mk::FileIoError());
    }

    SECTION("For a file containing UTF-8") {
        auto maybe_res =
            mk::slurpv<uint8_t>("./test/fixtures/text-with-utf8.txt");
        REQUIRE(!!maybe_res);
        std::vector<uint8_t> expect{
            'C', 'i', 'r', 'i',  0xc3, 0xa8, '\n',      // first line
            'A', 'g', 'l', 'i',  0xc3, 0xa8, '\n',      // second line
            'P', 'e', 'l', 0xc3, 0xa8, '\n',            // third line
            'P', 'e', 'r', 'c',  'h',  0xc3, 0xa9, '\n' // fourh line
        };
        REQUIRE(*maybe_res == expect);
        REQUIRE(maybe_res->size() == 28);
    }

    SECTION("For a purely binary file") {
        auto maybe_res = mk::slurpv<uint8_t>("./test/fixtures/gzipped.gz");
        REQUIRE(!!maybe_res);
        std::vector<uint8_t> expect{
            0x1f, 0x8b, 0x08, 0x08, 0xad, 0x82, 0x4d, 0x58,
            0x02, 0x03, 0x58, 0x4f, 0x00, 0xad, 0xce, 0xb1,

            0x4e, 0xc3, 0x30, 0x10, 0xc6, 0xf1, 0xdd, 0x4f,
            0x71, 0x23, 0x20, 0x5d, 0x1c, 0x40, 0x62, 0x88,

            0xc4, 0x80, 0x1a, 0x50, 0x17, 0x28, 0x82, 0x6c,
            0x88, 0xc1, 0xb1, 0x8f, 0xc6, 0xc8, 0xb1, 0x8d,

            0xef, 0x0a, 0xe2, 0xed, 0x69, 0x52, 0x44, 0x79,
            0x80, 0xdc, 0xfa, 0x49, 0xff, 0xdf, 0x75, 0x9b,

            0x76, 0xa3, 0xae, 0xf7, 0xa7, 0x54, 0x37, 0x10,
            0x13, 0x08, 0xb1, 0x30, 0xb0, 0xf8, 0x10, 0x20,

            0x12, 0x39, 0x90, 0x04, 0x3d, 0x81, 0x1f, 0x73,
            0xa0, 0x91, 0xa2, 0x90, 0x6b, 0x94, 0x3a, 0x83,

            0x97, 0xf6, 0xe1, 0x19, 0x56, 0x29, 0xb2, 0x67,
            0xa1, 0x68, 0xbf, 0x5f, 0x4f, 0x06, 0x91, 0xcc,

            0x8d, 0xd6, 0x5b, 0x2f, 0x5f, 0xd4, 0x57, 0x92,
            0x4a, 0x2e, 0xe9, 0x9d, 0xac, 0x54, 0xa9, 0x6c,

            0x75, 0x4a, 0xd1, 0x6b, 0xce, 0x64, 0xab, 0xfd,
            0xae, 0xfb, 0x90, 0x7a, 0xbd, 0xbe, 0xbd, 0x69,

            0x1b, 0x3d, 0x79, 0x38, 0x0d, 0xac, 0x85, 0xb1,
            0xae, 0x2f, 0xd0, 0x45, 0xb6, 0xc7, 0x70, 0x35,

            0xba, 0xd3, 0x19, 0x5c, 0x77, 0xdd, 0x23, 0x3c,
            0xd1, 0xc7, 0x6e, 0x7a, 0x70, 0x31, 0xee, 0x12,

            0xa7, 0x10, 0x96, 0xdf, 0xee, 0x51, 0x23, 0xe3,
            0xa8, 0xc0, 0x9d, 0xa7, 0xe0, 0xe0, 0xde, 0x44,

            0x9f, 0x77, 0xc1, 0x88, 0x4f, 0x71, 0x31, 0xf9,
            0x0a, 0x87, 0xd9, 0xc0, 0xb7, 0xc9, 0xc0, 0xf1,

            0x9f, 0xf1, 0xf7, 0xc5, 0xca, 0x64, 0xf1, 0x9f,
            0x04, 0x39, 0x15, 0x31, 0x61, 0x29, 0xfa, 0xbc,

            0x46, 0x7b, 0x08, 0xe3, 0x21, 0x3c, 0x7b, 0x3f,
            0x31, 0x51, 0x2e, 0xef, 0x0a, 0x02, 0x00, 0x00,
        };
        REQUIRE(*maybe_res == expect);
        REQUIRE(maybe_res->size() == 240);
    }
}

TEST_CASE("slurp() works as expected") {
    SECTION("In case of nonexistent file") {
        auto maybe_res = mk::slurp("/nonexistent");
        REQUIRE(!maybe_res);
        REQUIRE(maybe_res.as_error() == mk::FileIoError());
    }

    SECTION("In case of existent file") {
        auto maybe_res = mk::slurp("./test/fixtures/hosts.txt");
        std::string expect = "torproject.org\n"        // line
                             "ooni.nu\n"               // line
                             "neubot.org\n"            // line
                             "archive.org\n"           // line
                             "creativecommons.org\n"   // line
                             "cyber.law.harvard.edu\n" // line
                             "duckduckgo.com\n"        // line
                             "netflix.com\n"           // line
                             "nmap.org\n"              // line
                             "www.emule.com\n";
        REQUIRE(*maybe_res == expect);
    }
}

static size_t fwrite_fail(const void *, size_t, size_t, FILE *) { return 0; }

TEST_CASE("mk::overwrite_file() works as expected") {
    SECTION("If fopen() fails") {
        REQUIRE((mk::overwrite_file<fopen_fail>("xx", "xyz")) !=
                mk::NoError());
    }
    SECTION("If fwrite() fails") {
        REQUIRE((mk::overwrite_file<std::fopen, fwrite_fail>(
                      "xx", "xyz")) != mk::NoError());
    }
    SECTION("If fclose() fails") {
        REQUIRE((mk::overwrite_file<std::fopen, std::fwrite, fclose_fail>(
                      "xx", "xyz")) != mk::NoError());
    }
    SECTION("If everything is okay") {
        REQUIRE(mk::overwrite_file("xx", "xyz") == mk::NoError());
        REQUIRE(*mk::slurp("xx") == "xyz");
    }
    SECTION("Make sure file is actually overwritten") {
        REQUIRE(mk::overwrite_file("xx", "xyz") == mk::NoError());
        REQUIRE(mk::overwrite_file("xx", "abc") == mk::NoError());
        REQUIRE(*mk::slurp("xx") == "abc");
    }
}

TEST_CASE("mk::startswith() works as expected") {
    SECTION("For empty s and p") {
        REQUIRE(!!mk::startswith("", ""));
    }
    SECTION("For empty p") {
        REQUIRE(!!mk::startswith("antani", ""));
    }
    SECTION("For s shorter than p") {
        REQUIRE(!mk::startswith("x", "xyz"));
    }
    SECTION("For s equal to p") {
        REQUIRE(!!mk::startswith("xyz", "xyz"));
    }
    SECTION("For s longer than p") {
        REQUIRE(!!mk::startswith("antani", "ant"));
    }
    SECTION("For p present in s but not at s's beginning") {
        REQUIRE(!mk::startswith("antani", "nta"));
    }
}

TEST_CASE("mk::endswith() works as expected") {
    SECTION("For empty s and p") {
        REQUIRE(!!mk::endswith("", ""));
    }
    SECTION("For empty p") {
        REQUIRE(!!mk::endswith("antani", ""));
    }
    SECTION("For s shorter than p") {
        REQUIRE(!mk::endswith("z", "xyz"));
    }
    SECTION("For s equal to p") {
        REQUIRE(!!mk::endswith("xyz", "xyz"));
    }
    SECTION("For s longer than p") {
        REQUIRE(!!mk::endswith("antanix", "nix"));
    }
    // #TrueStory: this has been an embarassing bug
    SECTION("For p present in s but not at s's end") {
        REQUIRE(!mk::endswith("antanix", "tan"));
    }
}

TEST_CASE("random_choice isn't obviously wrong") {
    std::vector<std::string> choices = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k"};
    auto choice = mk::random_choice(choices);
    REQUIRE(std::find(choices.begin(), choices.end(), choice) != choices.end());
}

TEST_CASE("randomly_capitalize isn't obviously wrong") {
    auto lower_string = "abcdefghijklmnopqrstuvz";
    auto upper_string = "ABCDEFGHIJKLMNOPQRSTUVZ";
    auto rc_lower_string = mk::randomly_capitalize(lower_string);
    REQUIRE(rc_lower_string != lower_string);
    auto rc_upper_string = mk::randomly_capitalize(upper_string);
    REQUIRE(rc_upper_string != upper_string);
}


TEST_CASE("parse_iso8601_utc works as expected") {
    SECTION("For valid input") {
        std::tm t;
        auto e = mk::parse_iso8601_utc("2012-01-02T03:04:05Z", &t);
        REQUIRE(!e);
        REQUIRE(t.tm_sec == 5);
        REQUIRE(t.tm_min == 4);
        REQUIRE(t.tm_hour == 3);
        REQUIRE(t.tm_mday == 2);
        REQUIRE(t.tm_mon == 0);
        REQUIRE(t.tm_year == 112);
    }

    SECTION("For invalid input") {
        std::tm t;
        auto e = mk::parse_iso8601_utc("2012-01-02 03:04:05Z", &t);
        REQUIRE(!!e);
        REQUIRE(e == mk::ValueError());
    }

    SECTION("For no input") {
        std::tm t;
        auto e = mk::parse_iso8601_utc("", &t);
        REQUIRE(!!e);
        REQUIRE(e == mk::ValueError());
    }
}
