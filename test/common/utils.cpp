// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <cctype>
#include "../src/libmeasurement_kit/common/utils.hpp"

TEST_CASE("We are NOT using the default random seed") {
    // Note: the default random generator shall be seeded using 1 unless
    // seeded otherwise, according to Linux and MacOS manpages
    REQUIRE(mk::random_str(16) != "2PN0bdwPY7CA8M06");
}

TEST_CASE("random_within_charset() works with zero length string") {
    REQUIRE_THROWS_AS(mk::random_within_charset("", 16), mk::ValueError);
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
            REQUIRE(false);
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
            REQUIRE(false);
        }
    }
    REQUIRE(found_num);
    REQUIRE(found_upper);
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
