// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/net/evbuffer.hpp"
#include <event2/buffer.h>
#include <measurement_kit/net.hpp>

using namespace mk;
using namespace mk::net;

TEST_CASE("The constructor works correctly") {
    Buffer buffer;
    REQUIRE("" == buffer.read());
}

TEST_CASE("The constructor with null evbuffer works correctly") {
    evbuffer *evbuf = nullptr;
    Buffer buffer(evbuf);
    REQUIRE("" == buffer.read());
}

TEST_CASE("The constructor with nonnull evbuffer works correctly") {
    evbuffer *evbuf = evbuffer_new();
    REQUIRE(evbuf != nullptr);
    REQUIRE(evbuffer_add(evbuf, "foobar", 6) == 0);
    Buffer buffer(evbuf);
    REQUIRE("foobar" == buffer.read());
    evbuffer_free(evbuf);
}

TEST_CASE("The constructor with C++ string works correctly") {
    Buffer buffer("foobar");
    REQUIRE("foobar" == buffer.read());
}

TEST_CASE("The constructor with C string works correctly") {
    Buffer buffer("foobar", 6);
    REQUIRE("foobar" == buffer.read());
}

TEST_CASE("Insertion/extraction work correctly for evbuffer") {

    Buffer buff;
    Var<evbuffer> source = make_shared_evbuffer();
    Var<evbuffer> dest = make_shared_evbuffer();
    auto sa = std::string(65536, 'A');
    auto r = std::string();

    char data[65536];

    if (evbuffer_add(source.get(), sa.c_str(), sa.length()) != 0)
        throw std::runtime_error("evbuffer_add failed");

    SECTION("Insertion works correctly") {
        buff << source.get();
        REQUIRE(buff.length() == 65536);
        r = buff.read();
        REQUIRE(r == sa);
    }

    SECTION("Insertion throws for NULL evbuffer") {
        REQUIRE_THROWS(buff << (evbuffer *)nullptr);
    }

    SECTION("Extraction works correctly") {
        buff << source.get();
        buff >> dest.get();
        REQUIRE(buff.length() == 0);
        if (evbuffer_remove(dest.get(), data, sizeof(data)) != sizeof(data)) {
            throw std::runtime_error("evbuffer remove failed");
        }
        r = std::string(data, sizeof(data));
        REQUIRE(r == sa);
    }

    SECTION("Extraction throws for NULL evbuffer") {
        REQUIRE_THROWS(buff >> (evbuffer *)NULL);
    }
}

TEST_CASE("length() works correctly") {
    Buffer buff;

    SECTION("Lengh is zero at the beginning") { REQUIRE(buff.length() == 0); }

    SECTION("Lenght is X after we add X bytes of data") {
        buff << "0123456789";
        REQUIRE(buff.length() == 10);
    }

    SECTION("Length is zero after we add and then remove content") {
        buff << "0123456789";
        REQUIRE(buff.length() == 10);
        buff.discard();
        REQUIRE(buff.length() == 0);
    }
}

TEST_CASE("Foreach is robust to corner cases and errors") {
    Buffer buff;

    SECTION("No function is invoked when the buffer is empty") {
        buff.for_each([](const void *, size_t) {
            throw std::runtime_error("should not happen");
            return (false);
        });
    }

    /*
     * TODO: emulate libevent errors when the libevent emulation
     * layer is merged into the master branch.
     */
}

TEST_CASE("Foreach works correctly") {

    Buffer buff;
    auto counter = 0;
    auto r = std::string();

    /*
     * Initialize the source evbuffer.
     */

    Var<evbuffer> evbuf = make_shared_evbuffer();

    auto sa = std::string(512, 'A');
    auto sb = std::string(512, 'B');
    auto sc = std::string(512, 'C');
    auto expect = std::string();
    auto n_extents = 0;

    /* Repeat until we have three extents or more */
    do {
        if (evbuffer_add(evbuf.get(), sa.c_str(), sa.length()) != 0) {
            throw std::runtime_error("FAIL");
        }
        if (evbuffer_add(evbuf.get(), sb.c_str(), sb.length()) != 0) {
            throw std::runtime_error("FAIL");
        }
        if (evbuffer_add(evbuf.get(), sc.c_str(), sc.length()) != 0) {
            throw std::runtime_error("FAIL");
        }
        expect += sa + sb + sc;
    } while ((n_extents = evbuffer_peek(evbuf.get(), -1, NULL, NULL, 0)) < 3);

    SECTION("Make sure that we walk through all the extents") {

        buff << evbuf.get();
        buff.for_each([&](const void *p, size_t n) {
            r.append((const char *)p, n);
            ++counter;
            return (true);
        });

        // std::cerr << counter << std::endl;

        REQUIRE(counter == n_extents);
        REQUIRE(r == expect);
    }

    SECTION("Make sure that stopping early works as expected") {

        buff << evbuf.get();
        buff.for_each([&](const void *p, size_t n) {
            r.append((const char *)p, n);
            ++counter;
            return (false);
        });

        // std::cerr << r.length() << std::endl;

        REQUIRE(counter == 1);
        REQUIRE(expect.substr(0, r.length()) == r);
    }
}

TEST_CASE("Discard works correctly") {
    Buffer buff;

    SECTION("Discard does not misbehave when the buffer is empty") {
        buff.discard(1024);
        REQUIRE(buff.length() == 0);
    }

    SECTION("We can discard less than the total size") {
        buff.write_rand(32768);
        buff.discard(1024);
        REQUIRE(buff.length() == 32768 - 1024);
    }

    SECTION("We can discard exactly the total size") {
        buff.write_rand(32768);
        buff.discard(32768);
        REQUIRE(buff.length() == 0);
    }

    SECTION("Discard with no args discards all") {
        buff.write_rand(32768);
        buff.discard();
        REQUIRE(buff.length() == 0);
    }

    SECTION("No harm if we discard more than the total size") {
        buff.write_rand(32768);
        buff.discard(65535);
        REQUIRE(buff.length() == 0);
    }
}

TEST_CASE("Read works correctly") {
    Buffer buff;

    SECTION("Read does not misbehave when the buffer is empty") {
        auto s = buff.read(65535);
        REQUIRE(s.length() == 0);
    }

    SECTION("We can read less bytes than the total size") {
        buff.write_rand(32768);
        auto s = buff.read(1024);
        REQUIRE(buff.length() == 32768 - 1024);
        REQUIRE(s.length() == 1024);
    }

    SECTION("We can read exactly the total size") {
        buff.write_rand(32768);
        auto s = buff.read(32768);
        REQUIRE(buff.length() == 0);
        REQUIRE(s.length() == 32768);
    }

    SECTION("Read with no args reads all") {
        buff.write_rand(32768);
        auto s = buff.read();
        REQUIRE(buff.length() == 0);
        REQUIRE(s.length() == 32768);
    }

    SECTION("No harm if we ask more than the total size") {
        buff.write_rand(32768);
        auto s = buff.read(65535);
        REQUIRE(buff.length() == 0);
        REQUIRE(s.length() == 32768);
    }
}

TEST_CASE("Readn works correctly") {
    Buffer buff;

    SECTION("Readn does not misbehave when the buffer is empty") {
        auto s = buff.readn(65535);
        REQUIRE(s.length() == 0);
    }

    SECTION("We can readn less bytes than the total size") {
        buff.write_rand(32768);
        auto s = buff.readn(1024);
        REQUIRE(buff.length() == 32768 - 1024);
        REQUIRE(s.length() == 1024);
    }

    SECTION("We can readn exactly the total size") {
        buff.write_rand(32768);
        auto s = buff.readn(32768);
        REQUIRE(buff.length() == 0);
        REQUIRE(s.length() == 32768);
    }

    SECTION("Empty string returned if we ask more than the total size") {
        buff.write_rand(32768);
        auto s = buff.readn(65535);
        REQUIRE(buff.length() == 32768);
        REQUIRE(s.length() == 0);
    }
}

TEST_CASE("Readline works correctly") {
    Buffer buff;
    ErrorOr<std::string> line("");

    SECTION("We can read LF terminated lines") {
        buff << "HTTP/1.1 200 Ok\n"
             << "Content-Type: text/html\n"
             << "Content-Length: 7\n"
             << "\n"
             << "1234567";

        line = buff.readline(1024);
        REQUIRE(static_cast<bool>(line));
        REQUIRE(*line == "HTTP/1.1 200 Ok\n");

        line = buff.readline(1024);
        REQUIRE(static_cast<bool>(line));
        REQUIRE(*line == "Content-Type: text/html\n");

        line = buff.readline(1024);
        REQUIRE(static_cast<bool>(line));
        REQUIRE(*line == "Content-Length: 7\n");

        line = buff.readline(1024);
        REQUIRE(static_cast<bool>(line));
        REQUIRE(*line == "\n");

        // Here `line.as_value()` must be empty because
        // there is no ending LF.
        line = buff.readline(1024);
        REQUIRE(static_cast<bool>(line));
        REQUIRE(*line == "");
    }

    SECTION("We can read [CR]LF terminated lines") {
        buff << "HTTP/1.1 200 Ok\n"
             << "Content-Type: text/html\r\n"
             << "Content-Length: 7\n"
             << "\r\n"
             << "1234567";

        line = buff.readline(1024);
        REQUIRE(static_cast<bool>(line));
        REQUIRE(*line == "HTTP/1.1 200 Ok\n");

        line = buff.readline(1024);
        REQUIRE(static_cast<bool>(line));
        REQUIRE(*line == "Content-Type: text/html\r\n");

        line = buff.readline(1024);
        REQUIRE(static_cast<bool>(line));
        REQUIRE(*line == "Content-Length: 7\n");

        line = buff.readline(1024);
        REQUIRE(static_cast<bool>(line));
        REQUIRE(*line == "\r\n");

        // Here `line.as_value()` must be empty because
        // there is no ending LF.
        line = buff.readline(1024);
        REQUIRE(static_cast<bool>(line));
        REQUIRE(*line == "");
    }

    SECTION("EOL-not-found error is correctly reported") {
        buff << "HTTP/1.1 200 Ok";
        line = buff.readline(3);
        REQUIRE(!line);
        REQUIRE(line.as_error() == EOLNotFoundError());
        REQUIRE_THROWS_AS(*line, Error);
    }

    SECTION("Line-too-long error is correctly reported") {
        buff << "HTTP/1.1 200 Ok\n";
        line = buff.readline(3);
        REQUIRE(!line);
        REQUIRE(line.as_error() == LineTooLongError());
        REQUIRE_THROWS_AS(*line, Error);
    }
}

TEST_CASE("Write works correctly") {
    Buffer buff;
    auto pc = "0123456789";
    auto str = std::string(pc);

    SECTION("Writing a C++ string works") {
        buff.write(str);
        REQUIRE(buff.length() == 10);
        REQUIRE(buff.read() == str);
    }

    SECTION("Inserting a C++ string works") {
        buff << str;
        REQUIRE(buff.length() == 10);
        REQUIRE(buff.read() == str);
    }

    SECTION("Writing a C string works") {
        buff.write(pc);
        REQUIRE(buff.length() == 10);
        REQUIRE(buff.read() == str);
    }

    SECTION("Writing a NULL C string throws") {
        REQUIRE_THROWS(buff.write(NULL));
    }

    SECTION("Inserting a C string works") {
        buff << pc;
        REQUIRE(buff.length() == 10);
        REQUIRE(buff.read() == str);
    }

    SECTION("Inserting a NULL C string throws") {
        REQUIRE_THROWS(buff << (char *)NULL);
    }

    SECTION("Writing pointer-and-size works") {
        buff.write((void *)pc, 10);
        REQUIRE(buff.length() == 10);
        REQUIRE(buff.read() == str);
    }

    SECTION("Writing NULL pointer and size throws") {
        REQUIRE_THROWS(buff.write(NULL, 10));
    }

    SECTION("Writing random bytes works") {
        buff.write_rand(1048576);
        REQUIRE(buff.length() == 1048576);

        auto zeroes = 0, total = 0;
        buff.for_each([&](const void *pp, size_t n) {
            const char *p = (const char *)pp;
            for (size_t i = 0; i < n; ++i) {
                for (auto j = 0; j < 8; ++j) {
                    if ((p[i] & (1 << j)) == 0)
                        ++zeroes;
                    ++total;
                }
            }
            return (true);
        });
        double freq = zeroes / (double)total;

        /*
         * In a long-enough random sequence, we expect the number of
         * zeroes to be roughly equal to the number of ones.
         */
        REQUIRE(freq > 0.49);
        REQUIRE(freq < 0.51);
    }
}

TEST_CASE("Write into works correctly") {
    Buffer buff;

    SECTION("Typical usage") {
        buff.write(1024, [](void *buf, size_t cnt) {
            REQUIRE(buf != nullptr);
            REQUIRE(cnt == 1024);
            memset(buf, 0, cnt);
            return cnt;
        });
    }

    SECTION("Should throw if we use more than needed") {
        REQUIRE_THROWS(buff.write(1024, [](void *buf, size_t cnt) {
            REQUIRE(buf != nullptr);
            REQUIRE(cnt == 1024);
            memset(buf, 0, cnt);
            return cnt + 1;
        }));
    }

    SECTION("Should work OK if we use zero bytes") {
        buff.write(1024, [](void *buf, size_t cnt) {
            REQUIRE(buf != nullptr);
            REQUIRE(cnt == 1024);
            return 0;
        });
    }
}
