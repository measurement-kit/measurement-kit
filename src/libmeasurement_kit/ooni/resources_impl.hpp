// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <fstream>
#include <regex>

#include <measurement_kit/http.hpp>
#include <measurement_kit/ooni.hpp>

#include "../common/utils.hpp"

namespace mk {
namespace ooni {
namespace resources {

using namespace mk::http;

template <MK_MOCK_NAMESPACE(http, get)>
void get_latest_release_impl(Callback<Error, std::string> callback,
                             Settings settings, Var<Reactor> reactor,
                             Var<Logger> logger) {
    std::string url =
        "https://github.com/OpenObservatory/ooni-resources/releases/latest";
    /*
     * Make sure we are not redirected because we need to read the URL
     * to which github would like to redirect us.
     */
    settings["http/max_redirects"] = 0;
    logger->info("Downloading latest version; please, be patient...");
    http_get(url, [=](Error error, Var<Response> response) {
        if (error) {
            callback(error, "");
            return;
        }
        if (response->status_code / 100 != 3) {
            callback(CannotGetResourcesVersionError(), "");
            return;
        }
        if (response->headers.find("location") == response->headers.end()) {
            callback(MissingLocationHeaderError(), "");
            return;
        }
        std::string v = std::regex_replace(
            response->headers["location"],
            std::regex{
              "^https://github.com/OpenObservatory/ooni-resources/releases/tag/"
            },
            ""
        );
        logger->info("Latest resources version: %s", v.c_str());
        callback(NoError(), v);
    }, {}, settings, reactor, logger, nullptr, 0);
}

template <MK_MOCK_NAMESPACE(http, get)>
void get_manifest_as_json_impl(
        std::string latest, Callback<Error, nlohmann::json> callback,
        Settings settings, Var<Reactor> reactor, Var<Logger> logger) {
    std::string url{
      "https://github.com/OpenObservatory/ooni-resources/releases/download/"
    };
    url += latest;
    url += "/manifest.json";
    if (settings.find("http/max_redirects") == settings.end()) {
        settings["http/max_redirects"] = 4;
    }
    logger->info("Downloading manifest; please, be patient...");
    http_get(url, [=](Error error, Var<Response> response) {
        nlohmann::json result;
        if (error) {
            callback(error, result);
            return;
        }
        if (response->status_code != 200) {
            callback(CannotGetResourcesManifestError(), result);
            return;
        }
        try {
            result = nlohmann::json::parse(response->body);
        } catch (const std::invalid_argument &) {
            callback(JsonParseError(), result);
            return;
        }
        logger->info("Downloaded manifest");
        callback(NoError(), result);
    }, {}, settings, reactor, logger, nullptr, 0);
}

template <MK_MOCK_NAMESPACE(http, get)>
void get_resources_for_country_impl(std::string latest, nlohmann::json manifest,
                                    std::string country, Callback<Error> cb,
                                    Settings settings, Var<Reactor> reactor,
                                    Var<Logger> logger) {
    if (!manifest.is_object()) {
        reactor->call_soon([=]() { cb(JsonDomainError()); });
        return;
    }
    if (manifest.find("resources") == manifest.end()) {
        reactor->call_soon([=]() { cb(JsonKeyError()); });
        return;
    }
    if (settings.find("http/max_redirects") == settings.end()) {
        settings["http/max_redirects"] = 4;
    }
    // TODO: do we need to cross validate latest?
    std::vector<Continuation<Error>> input;
    /*
     * Important: MUST be entry and not &entry because we need a copy!
     */
    for (auto entry : manifest["resources"]) {
        input.push_back([=](Callback<Error> cb) {
            if (!entry.is_object()) {
                cb(JsonDomainError());
                return;
            }
            if (entry.find("country_code") == entry.end()) {
                cb(JsonKeyError());
                return;
            }
            std::string cc = entry["country_code"];
            if (country != cc and country != "ALL") {
                cb(NoError()); // Nothing to be done for this entry
                return;
            }
            if (entry.find("path") == entry.end()) {
                cb(JsonKeyError());
                return;
            }
            std::string path = entry["path"];
            path = std::regex_replace(path, std::regex{"/"}, ".");
            std::string url{"https://github.com/OpenObservatory/ooni-resources/"
                            "releases/download/"};
            url += latest;
            url += "/";
            url += path;
            http_get(url,
                     [=](Error error, Var<Response> response) {
                         if (error) {
                             cb(error);
                             return;
                         }
                         if (response->status_code != 200) {
                             cb(CannotGetResourceError());
                             return;
                         }
                         logger->info("Downloaded %s", path.c_str());
                         if (entry.find("sha256") == entry.end()) {
                             cb(JsonKeyError());
                             return;
                         }
                         if (sha256_of(response->body) != entry["sha256"]) {
                             cb(ResourceIntegrityError());
                             return;
                         }
                         logger->info("Verified %s", path.c_str());
                         std::ofstream ofile(path);
                         ofile << response->body;
                         ofile.close();
                         /*
                          * Note: bad() returns true if I/O fails.
                          */
                         if (ofile.bad()) {
                             cb(FileIoError());
                             return;
                         }
                         logger->info("Written %s", path.c_str());
                         cb(NoError());
                     },
                     {}, settings, reactor, logger, nullptr, 0);
        });
    }
    logger->info("Downloading resources; please, be patient...");
    mk::parallel(input, cb, 4);
}

} // namespace resources
} // namespace mk
} // namespace ooni
