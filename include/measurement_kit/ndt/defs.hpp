// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NDT_DEFS_HPP
#define MEASUREMENT_KIT_NDT_DEFS_HPP

// By default we pass MK_NDT_UPLOAD|MK_NDT_DOWNLOAD as settings["test_suite"]
// but you can tweak that by only requesting a single phase.
#define MK_NDT_MIDDLEBOX 1
#define MK_NDT_UPLOAD 2
#define MK_NDT_DOWNLOAD 4
#define MK_NDT_SIMPLE_FIREWALL 8
#define MK_NDT_STATUS 16
#define MK_NDT_META 32
#define MK_NDT_UPLOAD_EXT 64 // not implemented
#define MK_NDT_DOWNLOAD_EXT 128

#endif
