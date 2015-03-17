/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for net/buffer.hpp buffer.
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/common/libevent.hpp>
#include <ight/net/buffer.hpp>

using namespace ight::common::libevent;
using namespace ight::net::buffer;

TEST_CASE("The constructor works correctly", "[Buffer]") {
	REQUIRE_NOTHROW(Buffer());
}

TEST_CASE("Insertion/extraction work correctly for evbuffer") {

	Buffer buff;
	Evbuffer source;
	Evbuffer dest;
	auto sa = std::string(65536, 'A');
	auto r = std::string();

	char data[65536];

	if (evbuffer_add(source, sa.c_str(), sa.length()) != 0)
		throw std::runtime_error("evbuffer_add failed");

	SECTION("Insertion works correctly") {
		buff << source;
		REQUIRE(buff.length() == 65536);
		r = buff.read<char>();
		REQUIRE(r == sa);
	}

	SECTION("Insertion throws for NULL evbuffer") {
		REQUIRE_THROWS(buff << (evbuffer *) NULL);
	}

	SECTION("Extraction works correctly") {
		buff << source;
		buff >> dest;
		REQUIRE(buff.length() == 0);
		if (evbuffer_remove(dest, data, sizeof (data)) != sizeof (data))
			throw std::runtime_error("evbuffer remove failed");
		r = std::string(data, sizeof (data));
		REQUIRE(r == sa);
	}

	SECTION("Extraction throws for NULL evbuffer") {
		REQUIRE_THROWS(buff >> (evbuffer *) NULL);
	}
}

TEST_CASE("length() works correctly", "[Buffer]") {
	Buffer buff;

	SECTION("Lengh is zero at the beginning") {
		REQUIRE(buff.length() == 0);
	}

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

TEST_CASE("Foreach is robust to corner cases and errors", "[Buffer]") {
	Buffer buff;

	SECTION("No function is invoked when the buffer is empty") {
		buff.foreach([](evbuffer_iovec *) {
			throw std::runtime_error("should not happen");
			return (false);
		});
	}

	/*
	 * TODO: emulate libevent errors when the libevent emulation
	 * layer is merged into the master branch.
	 */
}

TEST_CASE("Foreach works correctly", "[Buffer]") {

	Buffer buff;
	auto counter = 0;
	auto r = std::string();

	/*
	 * Initialize the source evbuffer.
	 */

	Evbuffer evbuf;

	auto sa = std::string(512, 'A');
	auto sb = std::string(512, 'B');
	auto sc = std::string(512, 'C');
	auto expect = std::string();
	auto n_extents = 0;

	/* Repeat until we have three extents or more */
	do {
		if (evbuffer_add(evbuf, sa.c_str(), sa.length()) != 0)
			throw std::runtime_error("FAIL");
		if (evbuffer_add(evbuf, sb.c_str(), sb.length()) != 0)
			throw std::runtime_error("FAIL");
		if (evbuffer_add(evbuf, sc.c_str(), sc.length()) != 0)
			throw std::runtime_error("FAIL");
		expect += sa + sb + sc;
	} while ((n_extents = evbuffer_peek(evbuf, -1, NULL, NULL, 0)) < 3);

	SECTION("Make sure that we walk through all the extents") {

		buff << evbuf;
		buff.foreach([&](evbuffer_iovec *iov) {
			r.append((char *) iov->iov_base, iov->iov_len);
			++counter;
			return (true);
		});

		//std::cerr << counter << std::endl;

		REQUIRE(counter == n_extents);
		REQUIRE(r == expect);
	}

	SECTION("Make sure that stopping early works as expected") {

		buff << evbuf;
		buff.foreach([&](evbuffer_iovec *iov) {
			r.append((char *) iov->iov_base, iov->iov_len);
			++counter;
			return (false);
		});

		//std::cerr << r.length() << std::endl;

		REQUIRE(counter == 1);
		REQUIRE(expect.substr(0, r.length()) == r);
	}
}

TEST_CASE("Discard works correctly", "[Buffer]") {
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
		auto s = buff.read<char>(65535);
		REQUIRE(s.length() == 0);
	}

	SECTION("We can read less bytes than the total size") {
		buff.write_rand(32768);
		auto s = buff.read<char>(1024);
		REQUIRE(buff.length() == 32768 - 1024);
		REQUIRE(s.length() == 1024);
	}

	SECTION("We can read exactly the total size") {
		buff.write_rand(32768);
		auto s = buff.read<char>(32768);
		REQUIRE(buff.length() == 0);
		REQUIRE(s.length() == 32768);
	}

	SECTION("Read with no args reads all") {
		buff.write_rand(32768);
		auto s = buff.read<char>();
		REQUIRE(buff.length() == 0);
		REQUIRE(s.length() == 32768);
	}

	SECTION("No harm if we ask more than the total size") {
		buff.write_rand(32768);
		auto s = buff.read<char>(65535);
		REQUIRE(buff.length() == 0);
		REQUIRE(s.length() == 32768);
	}

	SECTION("Reading wide chars is not supported") {
		buff.write_rand(32768);
		REQUIRE_THROWS(buff.read<wchar_t>(1024));
	}
}

TEST_CASE("Readn works correctly") {
	Buffer buff;

	SECTION("Readn does not misbehave when the buffer is empty") {
		auto s = buff.readn<char>(65535);
		REQUIRE(s.length() == 0);
	}

	SECTION("We can readn less bytes than the total size") {
		buff.write_rand(32768);
		auto s = buff.readn<char>(1024);
		REQUIRE(buff.length() == 32768 - 1024);
		REQUIRE(s.length() == 1024);
	}

	SECTION("We can readn exactly the total size") {
		buff.write_rand(32768);
		auto s = buff.readn<char>(32768);
		REQUIRE(buff.length() == 0);
		REQUIRE(s.length() == 32768);
	}

	SECTION("Empty string returned if we ask more than the total size") {
		buff.write_rand(32768);
		auto s = buff.readn<char>(65535);
		REQUIRE(buff.length() == 32768);
		REQUIRE(s.length() == 0);
	}

	SECTION("Readn-ing wide chars is not supported") {
		buff.write_rand(32768);
		REQUIRE_THROWS(buff.readn<wchar_t>(1024));
	}
}

TEST_CASE("Readline works correctly", "[Buffer]") {
	Buffer buff;
	auto s = std::string();
	int error = 0;

	SECTION("We can read LF terminated lines") {
		buff << "HTTP/1.1 200 Ok\n"
		     << "Content-Type: text/html\n"
		     << "Content-Length: 7\n"
		     << "\n"
		     << "1234567";

		std::tie(error, s) = buff.readline(1024);
		REQUIRE(error == 0);
		REQUIRE(s == "HTTP/1.1 200 Ok\n");

		std::tie(error, s) = buff.readline(1024);
		REQUIRE(error == 0);
		REQUIRE(s == "Content-Type: text/html\n");

		std::tie(error, s) = buff.readline(1024);
		REQUIRE(error == 0);
		REQUIRE(s == "Content-Length: 7\n");

		std::tie(error, s) = buff.readline(1024);
		REQUIRE(error == 0);
		REQUIRE(s == "\n");

		/*
		 * Here `s` must be empty because there is no ending LF.
		 */
		std::tie(error, s) = buff.readline(1024);
		REQUIRE(error == 0);
		REQUIRE(s == "");
	}

	SECTION("We can read [CR]LF terminated lines") {
		buff << "HTTP/1.1 200 Ok\n"
		     << "Content-Type: text/html\r\n"
		     << "Content-Length: 7\n"
		     << "\r\n"
		     << "1234567";

		std::tie(error, s) = buff.readline(1024);
		REQUIRE(error == 0);
		REQUIRE(s == "HTTP/1.1 200 Ok\n");

		std::tie(error, s) = buff.readline(1024);
		REQUIRE(error == 0);
		REQUIRE(s == "Content-Type: text/html\r\n");

		std::tie(error, s) = buff.readline(1024);
		REQUIRE(error == 0);
		REQUIRE(s == "Content-Length: 7\n");

		std::tie(error, s) = buff.readline(1024);
		REQUIRE(error == 0);
		REQUIRE(s == "\r\n");

		/*
		 * Here `s` must be empty because there is no ending LF.
		 */
		std::tie(error, s) = buff.readline(1024);
		REQUIRE(error == 0);
		REQUIRE(s == "");
	}

}

TEST_CASE("Write works correctly", "[Buffer]") {
	Buffer buff;
	auto pc = "0123456789";
	auto str = std::string(pc);
	auto vect = std::vector<char>(str.begin(), str.end());

	SECTION("Writing a C++ string works") {
		buff.write(str);
		REQUIRE(buff.length() == 10);
		REQUIRE(buff.read<char>() == str);
	}

	SECTION("Inserting a C++ string works") {
		buff << str;
		REQUIRE(buff.length() == 10);
		REQUIRE(buff.read<char>() == str);
	}

	SECTION("Writing a C++ vector<char> works") {
		buff.write(vect);
		REQUIRE(buff.length() == 10);
		REQUIRE(buff.read<char>() == str);
	}

	SECTION("Inserting a C++ vector<char> works") {
		buff << vect;
		REQUIRE(buff.length() == 10);
		REQUIRE(buff.read<char>() == str);
	}

	SECTION("Writing a C string works") {
		buff.write(pc);
		REQUIRE(buff.length() == 10);
		REQUIRE(buff.read<char>() == str);
	}

	SECTION("Writing a NULL C string throws") {
		REQUIRE_THROWS(buff.write(NULL));
	}

	SECTION("Inserting a C string works") {
		buff << pc;
		REQUIRE(buff.length() == 10);
		REQUIRE(buff.read<char>() == str);
	}

	SECTION("Inserting a NULL C string throws") {
		REQUIRE_THROWS(buff << (char *) NULL);
	}

	SECTION("Writing pointer-and-size works") {
		buff.write((void *) pc, 10);
		REQUIRE(buff.length() == 10);
		REQUIRE(buff.read<char>() == str);
	}

	SECTION("Writing NULL pointer and size throws") {
		REQUIRE_THROWS(buff.write(NULL, 10));
	}

	SECTION("Writing random bytes works") {
		buff.write_rand(1048576);
		REQUIRE(buff.length() == 1048576);

		auto zeroes = 0, total = 0;
		buff.foreach([&](evbuffer_iovec *iov) {
			auto p = (char *) iov->iov_base;
			for (size_t i = 0; i < iov->iov_len; ++i) {
				for (auto j = 0; j < 8; ++j) {
					if ((p[i] & (1 << j)) == 0)
						++zeroes;
					++total;
				}
			}
			return (true);
		});
		double freq = zeroes / (double) total;

		/*
		 * In a long-enough random sequence, we expect the number of
		 * zeroes to be roughly equal to the number of ones.
		 */
		REQUIRE(freq > 0.49);
		REQUIRE(freq < 0.51);
	}
}
