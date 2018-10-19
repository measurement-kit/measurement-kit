#ifndef MEASUREMENT_KIT_LIBCURLX_LIBCURLX_H
#define MEASUREMENT_KIT_LIBCURLX_LIBCURLX_H
#ifdef __cplusplus
#include <memory>
extern "C" {
#endif

typedef struct mk_curlx_request mk_curlx_request_t;

mk_curlx_request_t *mk_curlx_request_new(void);

void mk_curlx_request_set_ca_path(mk_curlx_request_t *req, const char *p);

void mk_curlx_request_enable_http2(mk_curlx_request_t *req);

void mk_curlx_request_set_method_post(mk_curlx_request_t *req);

void mk_curlx_request_set_url(mk_curlx_request_t *req, const char *u);

void mk_curlx_request_add_header(mk_curlx_request_t *req, const char *h);

void mk_curlx_request_set_body(mk_curlx_request_t *req, const char *b);

void mk_curlx_request_set_timeout(mk_curlx_request_t *req, unsigned timeout);

void mk_curlx_request_set_proxy_url(mk_curlx_request_t *req, const char *u);

void mk_curlx_request_enable_follow_redirect(mk_curlx_request_t *req);

void mk_curlx_request_delete(mk_curlx_request_t *req);

typedef struct mk_curlx_response mk_curlx_response_t;

int mk_curlx_response_get_error(mk_curlx_response_t *res);

const char *mk_curlx_response_get_redirect_url(mk_curlx_response_t *res);

int mk_curlx_response_get_status_code(mk_curlx_response_t *res);

const char *mk_curlx_response_get_body(mk_curlx_response_t *res);

double mk_curlx_response_get_bytes_sent(mk_curlx_response_t *res);

double mk_curlx_response_get_bytes_recv(mk_curlx_response_t *res);

const char *mk_curlx_response_get_logs(mk_curlx_response_t *res);

const char *mk_curlx_response_get_request_headers(mk_curlx_response_t *res);

const char *mk_curlx_response_get_response_headers(mk_curlx_response_t *res);

const char *mk_curlx_response_get_certificate_chain(mk_curlx_response_t *res);

void mk_curlx_response_delete(mk_curlx_response_t *res);

mk_curlx_response_t *mk_curlx_perform(const mk_curlx_request_t *req);

#ifdef __cplusplus
}  // extern "C"

struct mk_curlx_request_deleter {
  void operator()(mk_curlx_request_t *req) {
    mk_curlx_request_delete(req);
  }
};

using mk_curlx_request_uptr = std::unique_ptr<mk_curlx_request_t,
                                              mk_curlx_request_deleter>;

struct mk_curlx_response_deleter {
  void operator()(mk_curlx_response_t *req) {
    mk_curlx_response_delete(req);
  }
};

using mk_curlx_response_uptr = std::unique_ptr<mk_curlx_response_t,
                                              mk_curlx_response_deleter>;

#ifdef MK_CURLX_INLINE_IMPL

#include <assert.h>

#include <sstream>
#include <string>
#include <vector>

#include <curl/curl.h>

struct mk_curlx_request {
  std::string ca_path;
  bool enable_http2 = false;
  bool method_post = false;
  std::string url;
  std::vector<std::string> headers;
  std::string body;
  unsigned timeout = 7;
  std::string proxy_url;
  bool follow_redir = false;
};

mk_curlx_request_t *mk_curlx_request_new() {
  return new mk_curlx_request_t{};
}

void mk_curlx_request_set_ca_path(mk_curlx_request_t *req, const char *p) {
  if (req != nullptr && p != nullptr) req->ca_path = p;
}

void mk_curlx_request_enable_http2(mk_curlx_request_t *req) {
  if (req != nullptr) req->enable_http2 = true;
}

void mk_curlx_request_set_method_post(mk_curlx_request_t *req) {
  if (req != nullptr) req->method_post = true;
}

void mk_curlx_request_set_url(mk_curlx_request_t *req, const char *u) {
  if (req != nullptr && u != nullptr) req->url = u;
}

void mk_curlx_request_add_header(mk_curlx_request_t *req, const char *h) {
  if (req != nullptr && h != nullptr) {
    req->headers.push_back(h);
  }
}

void mk_curlx_request_set_body(mk_curlx_request_t *req, const char *b) {
  if (req != nullptr && b != nullptr) req->body = b;
}

void mk_curlx_request_set_timeout(mk_curlx_request_t *req, unsigned timeout) {
  if (req != nullptr) req->timeout = timeout;
}

void mk_curlx_request_set_proxy_url(mk_curlx_request_t *req, const char *u) {
  if (req != nullptr && u != nullptr) req->proxy_url = u;
}

void mk_curlx_request_enable_follow_redirect(mk_curlx_request_t *req) {
  if (req != nullptr) req->follow_redir = true;
}

void mk_curlx_request_delete(mk_curlx_request_t *req) { delete req; }

struct mk_curlx_response {
  int error = CURLE_OK;
  std::string redirect_url;
  int status_code = 0;
  std::string body;
  double bytes_sent = 0.0;
  double bytes_recv = 0.0;
  std::string logs;
  std::string request_headers;
  std::string response_headers;
  std::string certs;
};

int mk_curlx_response_get_error(mk_curlx_response_t *res) {
  return (res != nullptr) ? res->error : CURLE_OK;
}

const char *mk_curlx_response_get_redirect_url(mk_curlx_response_t *res) {
  return (res != nullptr) ? res->redirect_url.c_str() : "";
}

int mk_curlx_response_get_status_code(mk_curlx_response_t *res) {
  return (res != nullptr) ? res->status_code : 200;
}

const char *mk_curlx_response_get_body(mk_curlx_response_t *res) {
  return (res != nullptr) ? res->body.c_str() : "";
}

double mk_curlx_response_get_bytes_sent(mk_curlx_response_t *res) {
  return (res != nullptr) ? res->bytes_sent : 0.0;
}

double mk_curlx_response_get_bytes_recv(mk_curlx_response_t *res) {
  return (res != nullptr) ? res->bytes_recv : 0.0;
}

const char *mk_curlx_response_get_logs(mk_curlx_response_t *res) {
  return (res != nullptr) ? res->logs.c_str() : "";
}

const char *mk_curlx_response_get_request_headers(mk_curlx_response_t *res) {
  return (res != nullptr) ? res->request_headers.c_str() : "";
}

const char *mk_curlx_response_get_response_headers(mk_curlx_response_t *res) {
  return (res != nullptr) ? res->response_headers.c_str() : "";
}

const char *mk_curlx_response_get_certificate_chain(mk_curlx_response_t *res) {
  return (res != nullptr) ? res->certs.c_str() : "";
}

void mk_curlx_response_delete(mk_curlx_response_t *res) { delete res; }

// mk_curlx_deleter is a custom deleter for a CURL handle.
struct mk_curlx_deleter {
  void operator()(CURL *handle) { curl_easy_cleanup(handle); }
};

// mk_curlx_uptr is a unique pointer to a CURL handle.
using mk_curlx_uptr = std::unique_ptr<CURL, mk_curlx_deleter>;

// mk_curlx_slist is a curl_slist with RAII semantic.
struct curlx_slist {
  curlx_slist() = default;
  curlx_slist(const curlx_slist &) = delete;
  curlx_slist &operator=(const curlx_slist &) = delete;
  curlx_slist(curlx_slist &&) = delete;
  curlx_slist &operator=(curlx_slist &&) = delete;
  ~curlx_slist() { curl_slist_free_all(p); }
  curl_slist *p = nullptr;
};

#ifndef MK_CURLX_EASY_INIT
// MK_CURLX_EASY_INIT allows to mock curl_easy_init
#define MK_CURLX_EASY_INIT curl_easy_init
#endif

#ifndef MK_CURLX_SLIST_APPEND
// MK_CURLX_SLIST_APPEND allows to mock curl_slist_append
#define MK_CURLX_SLIST_APPEND curl_slist_append
#endif

#ifndef MK_CURLX_EASY_SETOPT
// MK_CURLX_EASY_SETOPT allows to mock curl_easy_setopt
#define MK_CURLX_EASY_SETOPT curl_easy_setopt
#endif

#ifndef MK_CURLX_EASY_PERFORM
// MK_CURLX_EASY_PERFORM allows to mock curl_easy_perform
#define MK_CURLX_EASY_PERFORM curl_easy_perform
#endif

#ifndef MK_CURLX_EASY_GETINFO
// MK_CURLX_EASY_GETINFO allows to mock curl_easy_getinfo
#define MK_CURLX_EASY_GETINFO curl_easy_getinfo
#endif

extern "C" {

static size_t mk_curlx_body_cb(
    char *ptr, size_t size, size_t nmemb, void *userdata) noexcept {
  if (nmemb <= 0) {
    return 0;  // This means "no body"
  }
  if (size > SIZE_MAX / nmemb) {
    assert(false);  // If size is zero we we end up here
    return 0;
  }
  auto realsiz = size * nmemb;  // Overflow or zero not possible (see above)
  auto res = static_cast<mk_curlx_response_t *>(userdata);
  res->body += std::string{ptr, realsiz};
  // From fwrite(3): "[the return value] equals the number of bytes
  // written _only_ when `size` equals `1`". See also
  // https://sourceware.org/git/?p=glibc.git;a=blob;f=libio/iofwrite.c;h=800341b7da546e5b7fd2005c5536f4c90037f50d;hb=HEAD#l29
  return nmemb;
}

static int mk_curlx_debug_cb(CURL *handle,
                             curl_infotype type,
                             char *data,
                             size_t size,
                             void *userptr) {
  (void)handle;
  auto res = static_cast<mk_curlx_response_t *>(userptr);

  auto log_many_lines = [&](std::string prefix, const std::string &str) {
    std::stringstream ss;
    ss << str;
    std::string line;
    while (std::getline(ss, line, '\n')) {
      if (!prefix.empty()) {
        res->logs += prefix;
        res->logs += " ";
      }
      res->logs += line;
      res->logs += "\n";
    }
  };

  switch (type) {
    case CURLINFO_TEXT:
      log_many_lines("", std::string{(char *)data, size});
      break;
    case CURLINFO_HEADER_IN:
      {
        std::string s{(char *)data, size};
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
        std::string s{(char *)data, size};
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
      res->bytes_recv += (double)size;
      break;
    case CURLINFO_HEADER_OUT:
    case CURLINFO_DATA_OUT:
    case CURLINFO_SSL_DATA_OUT:
      res->bytes_sent += (double)size;
      break;
    case CURLINFO_TEXT:
    case CURLINFO_END:
      /* NOTHING */
      break;
  }

  return 0;
}

}  // extern "C"

// TODO(bassosimone):
//
// 1. Allow to disable CURLOPT_SSL_VERIFYPEER
//
// 2. Allow to disable CURLOPT_SSL_VERIFYHOST
//
// 3. Allow to set a specific SSL version with CURLOPT_SSLVERSION
mk_curlx_response_t *mk_curlx_perform(const mk_curlx_request_t *req) {
  if (req == nullptr) return nullptr;
  mk_curlx_response_uptr res{new mk_curlx_response_t{}};
  mk_curlx_uptr handle{MK_CURLX_EASY_INIT()};
  if (!handle) {
    res->error = CURLE_OUT_OF_MEMORY;
    res->logs += "curl_easy_init() failed\n";
    return res.release();
  }
  curlx_slist headers;
  for (auto &s : req->headers) {
    if ((headers.p = MK_CURLX_SLIST_APPEND(headers.p, s.c_str())) == nullptr) {
      res->error = CURLE_OUT_OF_MEMORY;
      res->logs += "curl_slist_append() failed\n";
      return res.release();
    }
  }
  if (!req->ca_path.empty() &&
      (res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_CAINFO,
                                         req->ca_path.c_str())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_CAINFO) failed\n";
    return res.release();
  }
  if (req->enable_http2 == true &&
      (res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_HTTP_VERSION,
                                         CURL_HTTP_VERSION_2_0)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_HTTP_VERSION) failed\n";
    return res.release();
  }
  if (headers.p != nullptr &&
      (res->error = MK_CURLX_EASY_SETOPT(
           handle.get(), CURLOPT_HTTPHEADER, headers.p)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_HTTPHEADER) failed\n";
    return res.release();
  }
  if (!req->body.empty() && req->method_post == true &&
      (res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_POSTFIELDS,
                                         req->body.c_str())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_POSTFIELDS) failed\n";
    return res.release();
  }
  if (req->method_post == true &&
      (res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_POST,
                                         1L)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_POST) failed\n";
    return res.release();
  }
  if ((res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_URL,
                                         req->url.c_str())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_URL) failed\n";
    return res.release();
  }
  if ((res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_WRITEFUNCTION,
                                         mk_curlx_body_cb)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_WRITEFUNCTION) failed\n";
    return res.release();
  }
  if ((res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_WRITEDATA,
                                         res.get())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_WRITEDATA) failed\n";
    return res.release();
  }
  if ((res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_TIMEOUT,
                                         req->timeout)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_TIMEOUT) failed\n";
    return res.release();
  }
  if ((res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_DEBUGFUNCTION,
                                         mk_curlx_debug_cb)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_DEBUGFUNCTION) failed\n";
    return res.release();
  }
  if ((res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_DEBUGDATA,
                                         res.get())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_DEBUGDATA) failed\n";
    return res.release();
  }
  if ((res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_VERBOSE,
                                         1L)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_VERBOSE) failed\n";
    return res.release();
  }
  if (!req->proxy_url.empty() &&
      (res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_PROXY,
                                         req->proxy_url.c_str())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_PROXY) failed\n";
    return res.release();
  }
  if (req->follow_redir == true &&
      (res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_FOLLOWLOCATION,
                                         1L)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_FOLLOWLOCATION) failed\n";
    return res.release();
  }
  if ((res->error = MK_CURLX_EASY_SETOPT(handle.get(), CURLOPT_CERTINFO,
                                         1L)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_CERTINFO) failed\n";
    return res.release();
  }
  if ((res->error = MK_CURLX_EASY_PERFORM(handle.get())) != CURLE_OK) {
    res->logs += "curl_easy_perform() failed\n";
    return res.release();
  }
  {
    long status_code = 0;
    if ((res->error = MK_CURLX_EASY_GETINFO(
             handle.get(), CURLINFO_RESPONSE_CODE, &status_code)) != CURLE_OK) {
      res->logs += "curl_easy_getinfo(CURLINFO_RESPONSE_CODE) failed\n";
      return res.release();
    }
    // In case the status code is clearly weird, normalize if for the cast
    // below to succeed and then pass it along. We assume that the caller is
    // checking it and is able to notice that something is wrong.
    if (status_code < 0 || status_code > INT_MAX) {
      status_code = INT_MAX;
    }
    res->status_code = (int)status_code;
  }
  {
    char *url = nullptr;
    if ((res->error = MK_CURLX_EASY_GETINFO(
             handle.get(), CURLINFO_REDIRECT_URL, &url)) != CURLE_OK) {
      res->logs += "curl_easy_getinfo(CURLINFO_REDIRECT_URL) failed\n";
      return res.release();
    }
    if (url != nullptr) res->redirect_url = url;
  }
  {
    curl_certinfo *certinfo = nullptr;
    if ((res->error = MK_CURLX_EASY_GETINFO(
             handle.get(), CURLINFO_CERTINFO, &certinfo)) != CURLE_OK) {
      res->logs += "curl_easy_getinfo(CURLINFO_CERTINFO) failed\n";
      return res.release();
    }
    if (certinfo != nullptr && certinfo->num_of_certs > 0) {
      for (int i = 0; i < certinfo->num_of_certs; i++) {
        for (auto slist = certinfo->certinfo[i]; slist; slist = slist->next) {
          if (slist->data != nullptr) {
            // This is a linked list with "key:value" strings. We change the
            // formar slightly so that parsing is easier.
            std::string s = slist->data;
            if (s.find("Cert:") == 0) {
              res->certs += s.substr(5);
            } else {
              res->certs += "# ";
              res->certs += s;
            }
            res->certs += "\n";
          }
        }
      }
    }
  }
  res->logs += "curl_easy_perform() success\n";
  return res.release();
}

#endif  // MK_CURLX_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_LIBCURLX_LIBCURLX_H
