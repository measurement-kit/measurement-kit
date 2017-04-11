// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_TRANSPORT_HPP
#define MEASUREMENT_KIT_NET_TRANSPORT_HPP

#include <measurement_kit/net/buffer.hpp>

namespace mk {
namespace net {

class TransportEmitter {
  public:
    virtual ~TransportEmitter();

    virtual void emit_connect() = 0;
    virtual void emit_data(Buffer buf) = 0;
    virtual void emit_flush() = 0;
    virtual void emit_error(Error err) = 0;

    virtual void on_connect(Callback<>) = 0;
    virtual void on_data(Callback<Buffer>) = 0;
    virtual void on_flush(Callback<>) = 0;
    virtual void on_error(Callback<Error>) = 0;

    virtual void close(Callback<>) = 0;
};

class TransportRecorder {
  public:
    virtual ~TransportRecorder();

    virtual void record_received_data() = 0;
    virtual void dont_record_received_data() = 0;
    virtual Buffer &received_data() = 0;

    virtual void record_sent_data() = 0;
    virtual void dont_record_sent_data() = 0;
    virtual Buffer &sent_data() = 0;
};

class TransportWriter {
  public:
    virtual ~TransportWriter();
    virtual void write(const void *, size_t) = 0;
    virtual void write(std::string) = 0;
    virtual void write(Buffer) = 0;
};

class TransportSocks5 {
  public:
    virtual ~TransportSocks5();
    virtual std::string socks5_address() = 0;
    virtual std::string socks5_port() = 0;
};

class TransportPollable {
  public:
    virtual ~TransportPollable();

    virtual void set_timeout(double) = 0;
    virtual void clear_timeout() = 0;

    /*
     * This is the interface with the underlying I/O system. As such, it is
     * specified here, for clarity, but is also protected.
     */
  protected:
    virtual void adjust_timeout(double) = 0;

    virtual void shutdown() = 0;

    /*
     * Writing is stopped automatically when the send buffer is empty
     * and, when this happens, the FLUSH event is emitted.
     */
    virtual void start_reading() = 0;
    virtual void stop_reading() = 0;
    virtual void start_writing() = 0;
};

class Transport : public TransportEmitter,
                  public TransportRecorder,
                  public TransportWriter,
                  public TransportSocks5,
                  public TransportPollable {
  public:
    virtual ~Transport();
};

/*
 *  Syntactic sugar when you need only to write or read (vis a vis Transport,
 *  required when you need read and write at the same time).
 */

void write(Var<Transport> txp, Buffer buf, Callback<Error> cb);

void readn(Var<Transport> txp, Var<Buffer> buff, size_t n, Callback<Error> cb,
           Var<Reactor> reactor = Reactor::global());

void read(Var<Transport> t, Var<Buffer> buff, Callback<Error> callback,
          Var<Reactor> reactor = Reactor::global());

} // namespace net
} // namespace mk
#endif
