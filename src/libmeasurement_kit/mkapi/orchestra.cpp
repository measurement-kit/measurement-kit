// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/internal/mkapi/orchestra.h>

#include <future>
#include <mutex>

#include "src/libmeasurement_kit/common/logger.hpp"
#include "src/libmeasurement_kit/ooni/orchestrate.hpp"

struct mkapi_orchestra_client {
  mk::ooni::orchestrate::Client client;
  std::string secrets_file;
  int64_t timeout = 30 /* seconds */;
};

struct mkapi_orchestra_result {
  uint64_t good = false;
  std::string logs;
  std::mutex mtxlock;
};

mkapi_orchestra_client_t *mkapi_orchestra_client_new() {
  return new mkapi_orchestra_client_t;
}

void mkapi_orchestra_client_set_available_bandwidth(
    mkapi_orchestra_client_t *client, const char *available_bandwidth) {
  if (client == nullptr || available_bandwidth == nullptr) {
    abort();
  }
  client->client.available_bandwidth = available_bandwidth;
}

void mkapi_orchestra_client_set_device_token(
    mkapi_orchestra_client_t *client, const char *device_token) {
  if (client == nullptr || device_token == nullptr) {
    abort();
  }
  client->client.device_token = device_token;
}

void mkapi_orchestra_client_set_ca_bundle_path(
    mkapi_orchestra_client_t *client, const char *ca_bundle_path) {
  if (client == nullptr || ca_bundle_path == nullptr) {
    abort();
  }
  client->client.ca_bundle_path = ca_bundle_path;
}

void mkapi_orchestra_client_set_geoip_country_path(
    mkapi_orchestra_client_t *client, const char *geoip_country_path) {
  if (client == nullptr || geoip_country_path == nullptr) {
    abort();
  }
  client->client.geoip_country_path = geoip_country_path;
}

void mkapi_orchestra_client_set_geoip_asn_path(
    mkapi_orchestra_client_t *client, const char *geoip_asn_path) {
  if (client == nullptr || geoip_asn_path == nullptr) {
    abort();
  }
  client->client.geoip_asn_path = geoip_asn_path;
}

void mkapi_orchestra_client_set_language(
    mkapi_orchestra_client_t *client, const char *language) {
  if (client == nullptr || language == nullptr) {
    abort();
  }
  client->client.language = language;
}

void mkapi_orchestra_client_set_network_type(
    mkapi_orchestra_client_t *client, const char *network_type) {
  if (client == nullptr || network_type == nullptr) {
    abort();
  }
  client->client.network_type = network_type;
}

void mkapi_orchestra_client_set_platform(
    mkapi_orchestra_client_t *client, const char *platform) {
  if (client == nullptr || platform == nullptr) {
    abort();
  }
  client->client.platform = platform;
}

void mkapi_orchestra_client_set_probe_asn(
    mkapi_orchestra_client_t *client, const char *probe_asn) {
  if (client == nullptr || probe_asn == nullptr) {
    abort();
  }
  client->client.probe_asn = probe_asn;
}

void mkapi_orchestra_client_set_probe_cc(
    mkapi_orchestra_client_t *client, const char *probe_cc) {
  if (client == nullptr || probe_cc == nullptr) {
    abort();
  }
  client->client.probe_cc = probe_cc;
}

void mkapi_orchestra_client_set_probe_family(
    mkapi_orchestra_client_t *client, const char *probe_family) {
  if (client == nullptr || probe_family == nullptr) {
    abort();
  }
  client->client.probe_family = probe_family;
}

void mkapi_orchestra_client_set_probe_timezone(
    mkapi_orchestra_client_t *client, const char *probe_timezone) {
  if (client == nullptr || probe_timezone == nullptr) {
    abort();
  }
  client->client.probe_timezone = probe_timezone;
}

void mkapi_orchestra_client_set_registry_url(
    mkapi_orchestra_client_t *client, const char *registry_url) {
  if (client == nullptr || registry_url == nullptr) {
    abort();
  }
  client->client.registry_url = registry_url;
}

void mkapi_orchestra_client_set_secrets_file(
    mkapi_orchestra_client_t *client, const char *secrets_file) {
  if (client == nullptr || secrets_file == nullptr) {
    abort();
  }
  client->secrets_file = secrets_file;
}

void mkapi_orchestra_client_set_software_name(
    mkapi_orchestra_client_t *client, const char *software_name) {
  if (client == nullptr || software_name == nullptr) {
    abort();
  }
  client->client.software_name = software_name;
}

void mkapi_orchestra_client_set_software_version(
    mkapi_orchestra_client_t *client, const char *software_version) {
  if (client == nullptr || software_version == nullptr) {
    abort();
  }
  client->client.software_version = software_version;
}

void mkapi_orchestra_client_add_supported_test(
    mkapi_orchestra_client_t *client, const char *supported_test) {
  if (client == nullptr || supported_test == nullptr) {
    abort();
  }
  client->client.supported_tests.push_back(supported_test);
}

void mkapi_orchestra_client_set_timeout(
    mkapi_orchestra_client_t *client, int64_t timeout) {
  if (client == nullptr) {
    abort();
  }
  client->timeout = timeout;
}

mkapi_orchestra_result_t *mkapi_orchestra_client_sync(
    const mkapi_orchestra_client_t *client) {
  if (client == nullptr) {
    abort();
  }
  mkapi_orchestra_result_uptr result{new mkapi_orchestra_result_t};
  // Operate on a copy of client to avoid any thread safety related risk
  mk::ooni::orchestrate::Client raii_client = client->client;
  if (client->timeout > 0) {
    raii_client.settings["net/timeout"] = client->timeout;
  }
  raii_client.logger->set_verbosity(MK_LOG_DEBUG);
  raii_client.logger->on_log([&result](uint32_t, const char *s) {
    if (s != nullptr) {
      std::unique_lock<std::mutex> _{result->mtxlock};  // Just in case
      result->logs += s;
      result->logs += "\n";
    }
  });
  mk::ooni::orchestrate::Auth auth;
  std::promise<mk::Error> promise;
  std::future<mk::Error> future = promise.get_future();
  // Assumption: if we can load the secrets path then we have
  // already registered the probe, otherwise we need to register
  // the probe and we don't need to call update afterwards.
  if (auth.load(client->secrets_file) != mk::NoError()) {
    raii_client.register_probe(
        mk::ooni::orchestrate::Auth::make_password(),
        [&client, &promise](mk::Error &&error, mk::ooni::orchestrate::Auth &&auth) {
          if (!error) {
            error = auth.dump(client->secrets_file);
          }
          promise.set_value(error);
        });
  } else {
    raii_client.update(
        std::move(auth),
        [&client, &promise](mk::Error &&error, mk::ooni::orchestrate::Auth &&auth) {
          if (!error) {
            error = auth.dump(client->secrets_file);
          }
          promise.set_value(error);
        });
  }
  result->good = (future.get() == mk::NoError());
  return result.release();
}

void mkapi_orchestra_client_delete(mkapi_orchestra_client_t *client) {
  delete client;
}

int64_t mkapi_orchestra_result_good(const mkapi_orchestra_result_t *result) {
  if (result == nullptr) {
    abort();
  }
  return result->good;
}

void mkapi_orchestra_result_get_binary_logs(
    const mkapi_orchestra_result_t *result,
    const uint8_t **base, size_t *count) {
  if (result == nullptr || base == nullptr || count == nullptr) {
    abort();
  }
  *base = (const uint8_t *)result->logs.c_str();
  *count = result->logs.size();
}

void mkapi_orchestra_result_delete(mkapi_orchestra_result_t *result) {
  delete result;
}

std::string mkapi_orchestra_result_moveout_logs(
    mkapi_orchestra_result_uptr &result) {
  if (result == nullptr) {
    abort();
  }
  return std::move(result->logs);
}
