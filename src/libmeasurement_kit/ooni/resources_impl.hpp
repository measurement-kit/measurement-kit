// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_RESOURCES_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_RESOURCES_IMPL_HPP

#include "src/libmeasurement_kit/common/mock.hpp"
#include "src/libmeasurement_kit/common/parallel.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/ooni/resources.hpp"

#include <fstream>
#include <regex>

#include "src/libmeasurement_kit/http/http.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {
namespace resources {

using namespace mk::http;

static inline std::string get_base_url(const Settings &settings) {
    return settings.get(
        "ooni/resources_base_url",
        std::string{
            "https://github.com/OpenObservatory/ooni-resources/releases/"});
}

static inline void set_max_redirects(Settings &settings) {
    if (settings.find("http/max_redirects") == settings.end()) {
        // Note: 20 is what is used by web_connectivity and by browsers
        settings["http/max_redirects"] = 20;
    }
}

static inline std::string sanitize_version(const std::string &s) {
    return std::regex_replace(s, std::regex{R"xx([\ \t\r\n]+)xx"}, "");
}

template <MK_MOCK_AS(http::get, http_get)>
void get_latest_release_impl(Callback<Error, std::string> callback,
                             Settings settings, SharedPtr<Reactor> reactor,
                             SharedPtr<Logger> logger) {
    /*
     * Note: this code does not use GitHub's "latest" release feature because
     * upon creating a new release that is marked as latest but it does not
     * contain yet all the files that are part of the release. For this reason
     * we create a "latest" tag containing a "version" file that we only
     * update when all the files are uploaded (#robustness). I'm adding this
     * note here because I was initially confused by how this works.
     *
     *      - Simone (2016-12-06)
     */
    set_max_redirects(settings);
    auto url = get_base_url(settings) + "download/latest/version";
    logger->info("Downloading latest version; please, be patient...");
    http_get(url, [=](Error error, SharedPtr<Response> response) {
        if (error) {
            callback(error, "");
            return;
        }
        if (response->status_code != 200) {
            callback(CannotGetResourcesVersionError(), "");
            return;
        }
        std::string v = sanitize_version(response->body);
        logger->info("Latest resources version: %s", v.c_str());
        callback(NoError(), v);
    }, {}, settings, reactor, logger, nullptr, 0);
}

template <MK_MOCK_AS(http::get, http_get)>
void get_manifest_as_json_impl(
        std::string latest, Callback<Error, Json> callback,
        Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    auto url = get_base_url(settings);
    url += "download/";
    url += latest;
    url += "/manifest.json";
    set_max_redirects(settings);
    logger->info("Downloading manifest; please, be patient...");
    http_get(url, [=](Error error, SharedPtr<Response> response) {
        Json result;
        if (error) {
            callback(error, result);
            return;
        }
        if (response->status_code != 200) {
            callback(CannotGetResourcesManifestError(), result);
            return;
        }
        try {
            result = Json::parse(response->body);
        } catch (const std::invalid_argument &) {
            callback(JsonParseError(), result);
            return;
        }
        logger->info("Downloaded manifest");
        callback(NoError(), result);
    }, {}, settings, reactor, logger, nullptr, 0);
}

static inline bool ostream_bad(const std::ostream &s) {
    return s.bad();
}

static inline std::string sanitize_path(const std::string &s) {
    return std::regex_replace(s, std::regex{R"xx([/\\]+)xx"}, ".");
}

template <MK_MOCK_AS(http::get, http_get), MK_MOCK(ostream_bad)>
void get_resources_for_country_impl(std::string latest, Json manifest,
                                    std::string country, Callback<Error> cb,
                                    Settings settings, SharedPtr<Reactor> reactor,
                                    SharedPtr<Logger> logger) {
    if (!manifest.is_object()) {
        reactor->call_soon([=]() { cb(JsonDomainError()); });
        return;
    }
    if (manifest.find("resources") == manifest.end()) {
        reactor->call_soon([=]() { cb(JsonKeyError()); });
        return;
    }
    set_max_redirects(settings);
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
            auto url = get_base_url(settings) + "download/";
            url += latest;
            url += "/";
            std::string path = sanitize_path(entry["path"]);
            url += path;
            http_get(url,
                     [=](Error error, SharedPtr<Response> response) {
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
                         /*
                          * TODO: When we integrate code to properly manage
                          * filepaths here we will need to make sure that we
                          * are not writing into bad places.
                          */
                         std::string dir = settings.get(
                            "ooni/resources_destdir",
                            std::string{"."}
                         );
                         /*
                          * See http://stackoverflow.com/questions/12971499
                          */
#if (defined _WIN32 || defined __CYGWIN__)
                         dir += R"xx(\)xx";
#else
                         dir += "/";
#endif
                         std::ofstream ofile(dir + path);
                         ofile << response->body;
                         ofile.close();
                         /*
                          * Note: bad() returns true if I/O fails.
                          */
                         if (ostream_bad(ofile)) {
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

template <MK_MOCK(get_manifest_as_json), MK_MOCK(get_resources_for_country)>
void get_resources_impl(std::string latest, std::string country,
                        Callback<Error> callback, Settings settings,
                        SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    get_manifest_as_json(latest,
                         [=](Error error, Json manifest) {
                             if (error) {
                                 callback(error);
                                 return;
                             }
                             get_resources_for_country(
                                 latest, manifest, country, callback, settings,
                                 reactor, logger);
                         },
                         settings, reactor, logger);
}

} // namespace resources
} // namespace mk
} // namespace ooni
#endif
