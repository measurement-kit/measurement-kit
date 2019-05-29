// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKCURL_HPP
#define MEASUREMENT_KIT_MKCURL_HPP

#include <stdint.h>

#include <string>
#include <vector>

namespace mk {
namespace curl {

/// Request is an HTTP request.
struct Request {
  /// ca_path is the path to the CA bundle to use.
  std::string ca_path;

  /// enable_http2 indicates whether we should enable HTTP2.
  bool enable_http2 = false;

  /// method is the method we want to use.
  std::string method = "GET";

  /// url is the URL we want to use.
  std::string url;

  /// headers contains the request headers.
  std::vector<std::string> headers;

  /// body contains the request body (possibly a binary body).
  std::string body;

  /// timeout is the time after which the request is aborted (in seconds). A
  /// value of zero means that no timeout is implemented.
  int64_t timeout = 0;

  /// proxy_url is the optional URL of the proxy to use.
  std::string proxy_url;

  /// enable_fastopen will enable TCP fastopen (if possible).
  bool enable_fastopen = false;

  /// follow_redir indicates whether we should follow redirects.
  bool follow_redir = false;

  /// connect_to is the string to pass to CURLOPT_CONNECT_TO. In the common
  /// case, you want to set this string to `::<IP>:`.
  std::string connect_to;

  /// retries tells this library how many times it needs to retry if
  /// a request fails because of a DNS error or a connect error. Note
  /// that the number here is the number of times a request will be
  /// _retried_, i.e., it does not count the initial request.
  size_t retries = 2;
};

/// Log is a log entry.
struct Log {
  /// msec is the number of milliseconds after which the specified event
  /// was logged computed using C++'s steady clock.
  int64_t msec = 0;

  /// line is the specific log line.
  std::string line;
};

/// Response is an HTTP response.
struct Response {
  /// error is the CURL error that occurred. In CURL this is an enum hence it
  /// is castable to int. Therefore using int64_t should always be okay. The
  /// value of zero is equivalent to CURLE_OK.
  int64_t error = 0;

  /// redirect_url is the URL to which we were redirected, if any.
  std::string redirect_url;

  /// status_code is the HTTP status code. In CURL this is a long hence
  /// using int64_t should always be wide enough.
  int64_t status_code = 0;

  /// body is the response body.
  std::string body;

  /// bytes_sent are the bytes sent when sending the request.
  int64_t bytes_sent = 0;

  /// bytes_recv are the bytes recv when receiving the response.
  int64_t bytes_recv = 0;

  // logs contains the (possibly non UTF-8) logs.
  std::vector<Log> logs;

  // request_headers contains the request line and the headers.
  std::string request_headers;

  // response_headers contains the response line and the headers.
  std::string response_headers;

  // certs contains a sequence of newline separated PEM certificates.
  std::string certs;

  // content_type is the response content type.
  std::string content_type;

  // http_version is the HTTP version.
  std::string http_version;
};

/// perform performs @p request and returns the Response.
Response perform(const Request &request) noexcept;

}  // namespace curl
}  // namespace mk

// If you just want to know about the API, you can stop reading here. What
// follows is the inline implementation of the library. By default it is not
// included when you include the header, but you can change this.
#ifdef MKCURL_INLINE_IMPL

#include <assert.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <sstream>

#include <curl/curl.h>

#include "mkmock.hpp"

// MKCURL_MOCK controls whether to enable mocking
#ifdef MKCURL_MOCK
#define MKCURL_HOOK MKMOCK_HOOK_ENABLED
#define MKCURL_HOOK_ALLOC MKMOCK_HOOK_ALLOC_ENABLED
#else
#define MKCURL_HOOK MKMOCK_HOOK_DISABLED
#define MKCURL_HOOK_ALLOC MKMOCK_HOOK_ALLOC_DISABLED
#endif

#ifndef MKCURL_ABORT
// MKCURL_ABORT allows to mock abort
#define MKCURL_ABORT abort
#endif

namespace mk {
namespace curl {

// mkcurl_log appends @p line to @p logs. It adds information on the current
// time in millisecond. It also appends a newline to the end of the line.
static void mkcurl_log(std::vector<Log> &logs, std::string &&line) {
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch());
  Log log;
  log.msec = now.count();
  std::swap(line, log.line);
  logs.push_back(std::move(log));
}

// mkcurl_deleter is a custom deleter for a CURL handle.
struct mkcurl_deleter {
  void operator()(CURL *handle) { curl_easy_cleanup(handle); }
};

// mkcurl_uptr is a unique pointer to a CURL handle.
using mkcurl_uptr = std::unique_ptr<CURL, mkcurl_deleter>;

// mkcurl_slist is a curl_slist with RAII semantic.
struct mkcurl_slist {
  // mkcurl_slist is the default constructor.
  mkcurl_slist() = default;
  // mkcurl_slist is the deleted copy constructor.
  mkcurl_slist(const mkcurl_slist &) = delete;
  // operator= is the deleted copy assignment.
  mkcurl_slist &operator=(const mkcurl_slist &) = delete;
  // mkcurl_slist is the deleted move constructor.
  mkcurl_slist(mkcurl_slist &&) = delete;
  // operator= is the deleted move assignment.
  mkcurl_slist &operator=(mkcurl_slist &&) = delete;
  // ~mkcurl_slist is the destructor.
  ~mkcurl_slist() { curl_slist_free_all(p); }
  // p is the pointer to the wrapped slist.
  curl_slist *p = nullptr;
};

}  // namespace curl
}  // namespace mk

extern "C" {

static size_t mkcurl_body_cb_(
    char *ptr, size_t size, size_t nmemb, void *userdata) {
  if (nmemb <= 0) {
    return 0;  // This means "no body"
  }
  if (size > SIZE_MAX / nmemb) {
    // If size is zero we end up into this branch.
    return 0;
  }
  if (ptr == nullptr || userdata == nullptr) {
    MKCURL_ABORT();
  }
  auto realsiz = size * nmemb;  // Overflow or zero not possible (see above)
  auto res = static_cast<mk::curl::Response *>(userdata);
  res->body += std::string{ptr, realsiz};
  // From fwrite(3): "[the return value] equals the number of bytes
  // written _only_ when `size` equals `1`". See also
  // https://sourceware.org/git/?p=glibc.git;a=blob;f=libio/iofwrite.c;h=800341b7da546e5b7fd2005c5536f4c90037f50d;hb=HEAD#l29
  return nmemb;
}

static int mkcurl_debug_cb_(CURL *handle,
                            curl_infotype type,
                            char *data,
                            size_t size,
                            void *userptr) {
  (void)handle;
  if (data == nullptr || userptr == nullptr) {
    MKCURL_ABORT();
  }
  auto res = static_cast<mk::curl::Response *>(userptr);

  auto log_many_lines = [&](std::string prefix, const std::string &str) {
    std::stringstream ss;
    ss << str;
    std::string line;
    while (std::getline(ss, line, '\n')) {
      std::stringstream logline;
      if (!prefix.empty()) {
        logline << prefix << " ";
      }
      logline << line;
      mkcurl_log(res->logs, logline.str());
    }
  };

  switch (type) {
    case CURLINFO_TEXT:
      log_many_lines("", std::string{(const char *)data, size});
      break;
    case CURLINFO_HEADER_IN:
      {
        std::string s{(const char *)data, size};
        log_many_lines("<", s);
        res->response_headers += s;
      }
      break;
    case CURLINFO_DATA_IN:
      log_many_lines("<data:", std::to_string(size));
      break;
    case CURLINFO_SSL_DATA_IN:
      log_many_lines("<tls_data:", std::to_string(size));
      break;
    case CURLINFO_HEADER_OUT:
      {
        std::string s{(const char *)data, size};
        log_many_lines(">", s);
        res->request_headers += s;
      }
      break;
    case CURLINFO_DATA_OUT:
      log_many_lines(">data:", std::to_string(size));
      break;
    case CURLINFO_SSL_DATA_OUT:
      log_many_lines(">tls_data:", std::to_string(size));
      break;
    case CURLINFO_END:
      /* NOTHING */
      break;
  }

  // Note regarding counting TLS data
  // ````````````````````````````````
  //
  // I am using the technique recommended by Stenberg on Stack Overflow [1]. It
  // was initially not clear to me whether cURL using OpenSSL counted the data
  // twice, once encrypted and once in clear text. However, using cURL using
  // OpenSSL on Linux and reading the source code [2] helped me to clarify that
  // it does indeed the right thing [3]. When using other TLS backends, it may
  // be that TLS data is not counted, but that's okay since we tell to users
  // that this is an estimate of the amount of used data.
  //
  // Notes
  // `````
  //
  // .. [1] https://stackoverflow.com/a/26905099
  //
  // .. [2] https://github.com/curl/curl/blob/6684653b/lib/vtls/openssl.c#L2295
  //
  // .. [3] the SSL function used is SSL_CTX_set_msg_callback which "[is] never
  //        [called for] application_data(23) because the callback will only be
  //        called for protocol messages" [4].
  //
  // .. [4] https://www.openssl.org/docs/man1.1.0/ssl/SSL_CTX_set_msg_callback.html
  switch (type) {
    case CURLINFO_HEADER_IN:
    case CURLINFO_DATA_IN:
    case CURLINFO_SSL_DATA_IN:
      // Implementation note: overflow is unlikely, so we don't care.
      res->bytes_recv += size;
      break;
    case CURLINFO_HEADER_OUT:
    case CURLINFO_DATA_OUT:
    case CURLINFO_SSL_DATA_OUT:
      // Implementation note: overflow is unlikely, so we don't care.
      res->bytes_sent += size;
      break;
    case CURLINFO_TEXT:
    case CURLINFO_END:
      /* NOTHING */
      break;
  }

  return 0;
}

}  // extern "C"

namespace mk {
namespace curl {

// HTTPVersionString returns a string representation of the cURL HTTP
// version string in @p httpv. If @p httpv has an unknown value, the
// return value is the empty string.
static const char *HTTPVersionString(long httpv) noexcept {
  switch (httpv) {
    case CURL_HTTP_VERSION_1_0:
      return "HTTP/1.0";
    case CURL_HTTP_VERSION_1_1:
      return "HTTP/1.1";
    case CURL_HTTP_VERSION_2_0:
      return "HTTP/2";
  }
  return "";
}

// perform_and_retry performs the request implied by @p handle for
// @p retries times. A request is only retried if (a) it failed and (b)
// the reason for failure is either DNS or connect error.
static CURLcode perform_and_retry(
    CURL *handlep, size_t retries, std::vector<Log> &logs) noexcept {
  CURLcode rv{};
  bool retriable{};
  for (;;) {
    rv = curl_easy_perform(handlep);
    MKCURL_HOOK(curl_easy_perform, rv);
    retriable = retries-- > 0 && (rv == CURLE_COULDNT_CONNECT ||
                                  rv == CURLE_COULDNT_RESOLVE_HOST);
    if (!retriable) {
      break;
    }
    mkcurl_log(logs, "Transient failure; let's try one more time");
  }
  return rv;
}

Response perform(const Request &req) noexcept {
  mkcurl_uptr handle;
  {
    CURL *handlep = curl_easy_init();
    MKCURL_HOOK_ALLOC(curl_easy_init, handlep, curl_easy_cleanup);
    handle.reset(handlep);
  }
  Response res;
  if (!handle) {
    res.error = CURLE_OUT_OF_MEMORY;
    mkcurl_log(res.logs, "curl_easy_init() failed");
    return res;
  }
  mkcurl_slist headers;  // This must have function scope
  for (auto &s : req.headers) {
    curl_slist *slistp = curl_slist_append(headers.p, s.c_str());
    MKCURL_HOOK_ALLOC(curl_slist_append_headers, slistp, curl_slist_free_all);
    if ((headers.p = slistp) == nullptr) {
      res.error = CURLE_OUT_OF_MEMORY;
      mkcurl_log(res.logs, "curl_slist_append() failed");
      return res;
    }
  }
  mkcurl_slist connect_to_settings;  // This must have function scope
  if (!req.connect_to.empty()) {
    curl_slist *slistp = curl_slist_append(
        connect_to_settings.p, req.connect_to.c_str());
    MKCURL_HOOK_ALLOC(
        curl_slist_append_connect_to, slistp, curl_slist_free_all);
    if ((connect_to_settings.p = slistp) == nullptr) {
      res.error = CURLE_OUT_OF_MEMORY;
      mkcurl_log(res.logs, "curl_slist_append() failed");
      return res;
    }
    res.error = curl_easy_setopt(handle.get(), CURLOPT_CONNECT_TO,
                                 connect_to_settings.p);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_CONNECT_TO, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_CONNECT_TO) failed");
      return res;
    }
  }
  if (req.enable_fastopen) {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_TCP_FASTOPEN, 1L);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_TCP_FASTOPEN, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_TCP_FASTOPEN) failed");
      return res;
    }
  }
  if (!req.ca_path.empty()) {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_CAINFO,
                                 req.ca_path.c_str());
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_CAINFO, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_CAINFO) failed");
      return res;
    }
  }
  if (req.enable_http2) {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_HTTP_VERSION,
                                 CURL_HTTP_VERSION_2_0);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_HTTP_VERSION, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_HTTP_VERSION) failed");
      return res;
    }
  }
  if (req.method == "POST" || req.method == "PUT") {
    // Disable sending `Expect: 100 continue`. There are actually good
    // arguments against NOT sending this specific HTTP header by default
    // with P{OS,U}T <https://curl.haxx.se/mail/lib-2017-07/0013.html>.
    {
      curl_slist *slistp = curl_slist_append(headers.p, "Expect:");
      MKCURL_HOOK_ALLOC(
          curl_slist_append_Expect_header, slistp, curl_slist_free_all);
      if ((headers.p = slistp) == nullptr) {
        res.error = CURLE_OUT_OF_MEMORY;
        mkcurl_log(res.logs, "curl_slist_append() failed");
        return res;
      }
    }
    {
      res.error = curl_easy_setopt(handle.get(), CURLOPT_POST, 1L);
      MKCURL_HOOK(curl_easy_setopt_CURLOPT_POST, res.error);
      if (res.error != CURLE_OK) {
        mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_POST) failed");
        return res;
      }
    }
    {
      res.error = curl_easy_setopt(handle.get(), CURLOPT_POSTFIELDS,
                                   req.body.c_str());
      MKCURL_HOOK(curl_easy_setopt_CURLOPT_POSTFIELDS, res.error);
      if (res.error != CURLE_OK) {
        mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_POSTFIELDS) failed");
        return res;
      }
    }
    // The following is very important to allow us to upload any kind of
    // binary file, otherwise CURL will use strlen(). We do not need to
    // send more than 2 GiB of data, hence we can safely limit ourself to
    // using CURLOPT_POSTFIELDSIZE that takes a `long` argument.
    {
      bool body_size_overflow = (req.body.size() > LONG_MAX);
      MKCURL_HOOK(body_size_overflow_inject, body_size_overflow);
      if (body_size_overflow) {
        mkcurl_log(res.logs, "Body larger than LONG_MAX");
        res.error = CURLE_FILESIZE_EXCEEDED;
        return res;
      }
      res.error = curl_easy_setopt(handle.get(), CURLOPT_POSTFIELDSIZE,
                                   (long)req.body.size());
      MKCURL_HOOK(curl_easy_setopt_CURLOPT_POSTFIELDSIZE, res.error);
      if (res.error != CURLE_OK) {
        mkcurl_log(res.logs, "curl_easy_setopt(MKCURLOPT_POSTFIELDSIZE) failed");
        return res;
      }
    }
    if (req.method == "PUT") {
      res.error = curl_easy_setopt(handle.get(), CURLOPT_CUSTOMREQUEST, "PUT");
      MKCURL_HOOK(curl_easy_setopt_CURLOPT_CUSTOMREQUEST, res.error);
      if (res.error) {
        mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_CUSTOMREQUEST) failed");
        return res;
      }
    }
  } else if (req.method != "GET") {
    res.error = CURLE_BAD_FUNCTION_ARGUMENT;
    mkcurl_log(res.logs, "unsupported request method");
    return res;
  }
  if (headers.p != nullptr) {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_HTTPHEADER, headers.p);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_HTTPHEADER, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_HTTPHEADER) failed");
      return res;
    }
  }
  {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_URL, req.url.c_str());
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_URL, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_URL) failed");
      return res;
    }
  }
  {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_WRITEFUNCTION,
                                 mkcurl_body_cb_);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_WRITEFUNCTION, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_WRITEFUNCTION) failed");
      return res;
    }
  }
  {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_WRITEDATA, &res);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_WRITEDATA, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_WRITEDATA) failed");
      return res;
    }
  }
  // CURL uses MSG_NOSIGNAL where available (i.e. Linux) and SO_NOSIGPIPE
  // where available (i.e. BSD). This covers all the UNIX operating systems
  // that we care about (Android, Linux, iOS, macOS). We additionally need
  // to avoid signals because we are acting as a library that is integrated
  // into several different languages, so stealing the signal handler from
  // the language MAY have a negative impact. However, disabling signal
  // handlers will also make the getaddrinfo() resolver in cURL block until
  // getaddrinfo() returns, unless we're using the threaded or the c-ares
  // DNS backend. Since the threaded resolver should now be used in most
  // Unix distros, we need mainly to remember to enable it when we're cross
  // compiling cURL in measurement-kit/script-build-unix.
  {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_NOSIGNAL, 1L);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_NOSIGNAL, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_NOSIGNAL) failed");
      return res;
    }
  }
  {
    long t = 0L; // Note: `0L` means "infinite" for CURLOPT_TIMEOUT.
    if (req.timeout >= 0 && req.timeout < LONG_MAX) t = (long)req.timeout;
    res.error = curl_easy_setopt(handle.get(), CURLOPT_TIMEOUT, t);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_TIMEOUT, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_TIMEOUT) failed");
      return res;
    }
  }
  {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_DEBUGFUNCTION,
                                 mkcurl_debug_cb_);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_DEBUGFUNCTION, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_DEBUGFUNCTION) failed");
      return res;
    }
  }
  {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_DEBUGDATA, &res);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_DEBUGDATA, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_DEBUGDATA) failed");
      return res;
    }
  }
  {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_VERBOSE, 1L);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_VERBOSE, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_VERBOSE) failed");
      return res;
    }
  }
  if (!req.proxy_url.empty()) {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_PROXY,
                                 req.proxy_url.c_str());
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_PROXY, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_PROXY) failed");
      return res;
    }
  }
  if (req.follow_redir) {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_FOLLOWLOCATION, 1L);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_FOLLOWLOCATION, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_FOLLOWLOCATION) failed");
      return res;
    }
  }
  {
    res.error = curl_easy_setopt(handle.get(), CURLOPT_CERTINFO, 1L);
    MKCURL_HOOK(curl_easy_setopt_CURLOPT_CERTINFO, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_setopt(CURLOPT_CERTINFO) failed");
      return res;
    }
  }
  {
    res.error = perform_and_retry(handle.get(), req.retries, res.logs);
    if (res.error != CURLE_OK) {
      return res;
    }
  }
  {
    long status_code = 0;
    res.error = curl_easy_getinfo(
        handle.get(), CURLINFO_RESPONSE_CODE, &status_code);
    MKCURL_HOOK(curl_easy_getinfo_CURLINFO_RESPONSE_CODE, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_getinfo(CURLINFO_RESPONSE_CODE) failed");
      return res;
    }
    res.status_code = (int64_t)status_code;
  }
  {
    char *url = nullptr;
    res.error = curl_easy_getinfo(handle.get(), CURLINFO_REDIRECT_URL, &url);
    MKCURL_HOOK(curl_easy_getinfo_CURLINFO_REDIRECT_URL, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_getinfo(CURLINFO_REDIRECT_URL) failed");
      return res;
    }
    if (url != nullptr) res.redirect_url = url;
  }
  {
    curl_certinfo *certinfo = nullptr;
    res.error = curl_easy_getinfo(handle.get(), CURLINFO_CERTINFO, &certinfo);
    MKCURL_HOOK(curl_easy_getinfo_CURLINFO_CERTINFO, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_getinfo(CURLINFO_CERTINFO) failed");
      return res;
    }
    if (certinfo != nullptr && certinfo->num_of_certs > 0) {
      for (int i = 0; i < certinfo->num_of_certs; i++) {
        for (auto slist = certinfo->certinfo[i]; slist; slist = slist->next) {
          if (slist->data != nullptr) {
            // Just pass in the certificates and ignore the rest.
            std::string s = slist->data;
            if (s.find("Cert:") == 0) {
              res.certs += s.substr(5);
              res.certs += "\n";
            }
          }
        }
      }
    }
  }
  {
    char *ct = nullptr;
    res.error = curl_easy_getinfo(handle.get(), CURLINFO_CONTENT_TYPE, &ct);
    MKCURL_HOOK(curl_easy_getinfo_CURLINFO_CONTENT_TYPE, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_getinfo(CURLINFO_CONTENT_TYPE) failed");
      return res;
    }
    if (ct != nullptr) res.content_type = ct;
  }
  {
    long httpv = 0L;
    res.error = curl_easy_getinfo(handle.get(), CURLINFO_HTTP_VERSION, &httpv);
    MKCURL_HOOK(curl_easy_getinfo_CURLINFO_HTTP_VERSION, res.error);
    if (res.error != CURLE_OK) {
      mkcurl_log(res.logs, "curl_easy_getinfo(CURLINFO_HTTP_VERSION) failed");
      return res;
    }
    res.http_version = HTTPVersionString(httpv);
  }
  return res;
}

}  // namespace curl
}  // namespace mk
#endif  // MKCURL_INLINE_IMPL
#endif  // MEASUREMENT_KIT_MKCURL_HPP
