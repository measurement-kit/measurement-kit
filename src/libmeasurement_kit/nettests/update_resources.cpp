// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

UpdateResourcesTask::UpdateResourcesTask() : BaseTest() {
    runnable.reset(new UpdateResourcesRunnable);
    runnable->test_name = "update_resources";
    runnable->test_version = "0.0.1";
    runnable->options["no_file_report"] = 1;
    runnable->options["no_collector"] = 1;
}

void UpdateResourcesRunnable::main(std::string, Settings options,
                                   Callback<Var<report::Entry>> cb) {
    /*
     * XXX: this implementation of update-resources MAY download specific
     * files even when they are not modified because it doesn't honour the
     * version number of the resource. To be fixed in v0.6.
     *
     * TODO: implement conversion from Error to Var<report::Entry>.
     */
    logger->progress(0.33, "get latest release...");
    ooni::resources::get_latest_release(
        [=](Error error, std::string latest) {
            if (error) {
                cb(nullptr);
                return;
            }
            ErrorOr<double> latest_number = lexical_cast<double>(latest);
            if (!latest_number) {
                cb(nullptr);
                return;
            }
            std::string manifest_path =
                options.get("resource_manifest_path",
                            std::string{"resource-manifest.json"});
            double current_version = 0.0;
            do {
                ErrorOr<std::string> old_manifest_data = slurp(manifest_path);
                if (!old_manifest_data) {
                    break;
                }
                nlohmann::json old_manifest;
                try {
                    old_manifest = nlohmann::json::parse(*old_manifest_data);
                } catch (const std::exception &) {
                    break;
                }
                try {
                    current_version = old_manifest["version"];
                } catch (const std::exception &) {
                    break;
                }
            } while (0);
            logger->info("Latest: %f; current: %f", *latest_number,
                         current_version);
            if (*latest_number <= current_version) {
                cb(nullptr);
                return;
            }
            logger->progress(0.66, "get latest manifest");
            ooni::resources::get_manifest_as_json(
                latest,
                [=](Error error, nlohmann::json new_manifest) {
                    if (error) {
                        cb(nullptr);
                        return;
                    }
                    logger->progress(0.99, "get resources for country");
                    std::string country = probe_cc;
                    if (probe_cc == "ZZ") {
                        country = "ALL";
                    }
                    ooni::resources::get_resources_for_country(
                        latest, new_manifest, country,
                        [=](Error error) {
                            if (error) {
                                cb(nullptr);
                                return;
                            }
                            std::ofstream ofile(manifest_path);
                            ofile << new_manifest.dump(4) << "\n";
                            if (!ofile) {
                                cb(nullptr);
                                return;
                            }
                            cb(nullptr);
                        },
                        options, reactor, logger);
                },
                options, reactor, logger);
        },
        options, reactor, logger);
}

} // namespace nettests
} // namespace mk
