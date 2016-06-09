// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_WEB_CONNECTIVITY_HPP
#define SRC_OONI_WEB_CONNECTIVITY_HPP

#include <measurement_kit/ext.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/ooni.hpp>

#include <measurement_kit/net/transport.hpp>

#include "src/ooni/tcp_test_impl.hpp"
#include "src/ooni/dns_test_impl.hpp"
#include "src/ooni/http_test_impl.hpp"
#include "src/ooni/ooni_test_impl.hpp"

#include <iostream>
#include <string>
#include <unistd.h>

using json = nlohmann::json;

namespace mk {
namespace ooni {

typedef std::vector<std::pair<std::string, int>> SocketList;



class WebConnectivityTestImpl : public TCPTestImpl,
                                public DNSTestImpl,
                                public HTTPTestImpl {

    public:
      WebConnectivityTestImpl(std::string input_filepath_, Settings options_)
          : ooni::OoniTestImpl(input_filepath_, options_) {
          test_name = "web_connectivity";
          test_version = "0.0.1";
      }

      void compare_http_requests(Var<report::Entry> entry,
              Entry experiment_response, Entry control) {
        // XXX fill me in
        (*entry)["body_length_match"] = nullptr;
        (*entry)["status_code_match"] = nullptr;
        (*entry)["headers_match"] = nullptr;
      }

      void compare_dns_queries(Var<report::Entry> entry,
              Entry experiment_queries, Entry control) {
        // XXX fill me in
        (*entry)["dns_consistency"] = nullptr;
      }

      void compare_tcp_connect(Var<report::Entry> entry,
              Entry experiment_queries, Entry control) {
        // XXX fill me in
        // (*entry)["tcp_connect"][idx]["status"]["blocked"] = nullptr;
      }

      void compare_control_experiment(Var<report::Entry> entry) {
          if ((*entry)["http_experiment_failure"] == nullptr) {
            compare_http_requests(
                    entry,
                    (*entry)["requests"][0]["response"],
                    (*entry)["control"]["http_request"]
            );
          }

          if ((*entry)["dns_experiment_failure"] == nullptr) {
              compare_dns_queries(
                entry,
                (*entry)["tcp_connect"],
                (*entry)["control"]["dns"]
              )
          }

          compare_tcp_connect(entry, (*entry)["queries"], (*entry)["control"]["tcp_connect"]);
      }

      void control_request(Var<report::Entry> entry,
              Settings options,
              SocketList socket_list, std::string url,
              Callback<Error> callback) {
          Settings request_settings;
          http::Headers headers;
          json request;
          for (auto socket : socket_list) {
            // Formats the sockets as IP:PORT
            std::ostringstream ss;
            ss << socket->first;
            ss << ":";
            ss << socket->second;
            request["tcp_connect"].push_back(ss.str());
          }
          request["http_request"] = url;
          std::string body = request.dump();

          request_settings["http/url"] = options["backend/address"];
          request_settings["http/method"] = "POST";
          headers["Content-Type"] = "application/json";

          if (options["backend/type"] == "cloudfront") {
            // XXX set the appropriate headers to support cloud-fronting.
          }

          http::request(request_settings, headers, body,
                  [=](Error error, http::Response response) {
            if (!error) {
              try {
                (*entry)["control"] = json::parse(response->body);
                callback(nullptr);
                return;
              } catch (const std::invalid_argument &) {
                (*entry)["control_failure"] = "json_parse_error";
                callback(JsonParseError());
                return;
              }
            }
            (*entry)["control_failure"] = error.as_ooni_error();
            callback(error);
            return;
          });
      }

      void experiment_http_request(Var<report::Entry> entry,
              std::string url, Callback<Error> cb) {
        http::Headers headers;
        Settings options;
        std::string body;
        options["url"] = url;

        request(entry, options, headers, body, [=](Error err,
                    http::Response response) {
          if (err) {
            (*entry)["http_experiment_failure"] = err.as_ooni_error();
            cb(err);
            return;
          }
          cb();
        });
      }

      void experiment_tcp_connect(Var<report::Entry> entry,
              SocketList sockets,
              Callback<Error> cb) {

        int socket_count = sockets.size();
        int sockets_tested = 0;

        auto handle_response = [=](Error err, Var<net::Transport> txp) {
          sockets_tested += 1;
          json result = {
              {"ip": socket->first},
              {"port": socket->second},
              {"status":
                  {"success": nullptr},
                  {"failure": nullptr},
                  {"blocked": nullptr}
              },
          }
          if (err) {
              result["status"]["success"] = false;
              result["status"]["failure"] = err.as_ooni_error();
          } else {
              result["status"]["success"] = true;
              result["status"]["blocked"] = false;
          }
          (*entry)["tcp_connect"].push_back(result);
          if (socket_count == sockets_tested) {
            txp->close([=]{
              cb(*entry);
            });
          } else {
            // XXX we make optimistic closing
            txp->close([=]{});
          }
        }

        for (auto socket : sockets) {
            std::string address = socket->first;
            int port = socket->second;
            Settings connect_options;
            connect_options["host"] = socket->first
            connect_options["port"] = socket->second
            connect(options, handle_connect);
        }
      }

      void experiment_dns_query(Var<report::Entry> entry,
              std::string hostname,
              std::string nameserver,
              Callback<Error, std::vector<std::string>> callback) {

        std::vector<std::string> addresses;
        // XXX need to change the definition of query to pass along the error
        // as well.
        query(entry, "A", "IN", hostname, nameserver,
              [=](dns::Message message) {
                for (auto answer : message.answers) {
                  // XXX need to change the query function to support passing
                  // along the error and add support for CNAME.
                  addresses.push_back(answer.ipv4);
                }
                cb(nullptr, addresses);
              }, options);
      }

      void main(std::string input, Settings options,
                Callback<report::Entry> cb) {
          Var<report::Entry> entry(new report::Entry);
          // XXX
          (*entry)["client_resolver"] = nullptr;
          (*entry)["retries"] = nullptr;

          (*entry)["dns_consistency"] = nullptr;
          (*entry)["body_length_match"] = nullptr;
          (*entry)["headers_match"] = nullptr;
          (*entry)["status_code_match"] = nullptr;

          (*entry)["accessible"] = nullptr;
          (*entry)["blocking"] = nullptr;

          (*entry)["control_failure"] = nullptr;
          (*entry)["http_experiment_failure"] = nullptr;
          (*entry)["dns_experiment_failure"] = nullptr;

          (*entry)["tcp_connect"] = json::array();
          (*entry)["control"] = json({});

          ErrorOr<http::Url> url =
            mk::http::parse_url_noexcept(input);

          if (!url) {
            logger->debug("Invalid test url.");
            cb(*entry);
            return;
          }

          // XXX check if this hostname is actually a hostname and not an IP
          std::string hostname = url.address;
          // XXX if this option is not specified detect the system resolver
          std::string nameserver = options["nameserver"];

          experiment_dns_query(entry, hostname, nameserver, [=](Error err,
                    std::vector<std::string> addresses) {

            experiment_tcp_connect(entry, addresses, [=](Error err){

              experiment_http_request(entry, input, [=](Error err){

                control_request(entry, addresses, input, [=](Error err) {

                  compare_control_experiment(entry);
                  callback(entry);

                });

              }); // end http_request

            }); // end tcp_connect

          }); // end dns_query
      }

};

} // namespace ooni
} // namespace mk
#endif
