# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

cmake_minimum_required(VERSION 3.6)

# Note: CMake is mainly meant for usage under Windows, while under Unix we
# prefer to use autotools; hence assume for Unix this file has quirks.

set(CMAKE_CXX_STANDARD 14)
file(
     GLOB
     sources
     "src/libmeasurement_kit/*/*.c"
     "src/libmeasurement_kit/*/*.cpp"
    )

add_definitions(-DMK_CA_BUNDLE="mk-ca-bundle.pem")

if(APPLE)
  set(openssl_include_path /usr/local/Cellar/openssl/1.0.2h_1/include)
  set(osx_extra_libs event_pthreads event_openssl)
  set(osx_link_path /usr/local/Cellar/openssl/1.0.2h_1/lib)
endif()

if(MSVC)
  set(msvc_include_path ${CMAKE_SOURCE_DIR}/build/windows64/include)
  set(msvc_link_path ${CMAKE_SOURCE_DIR}/build/windows64/lib)
endif()

add_library(measurement_kit_static STATIC ${sources})
target_include_directories(
                           measurement_kit_static
                           PUBLIC
                           ${openssl_include_path}
                           ${msvc_include_path}
                           ${CMAKE_SOURCE_DIR}/include
                          )

if(MSVC)
  set(libevent_static_lib_name libevent)
  set(msvc_extra_libs ws2_32)
else()
  set(libevent_static_lib_name event)
endif()

link_directories(${msvc_link_path} ${osx_link_path})
add_executable(
	       measurement_kit
	       ${CMAKE_SOURCE_DIR}/src/measurement_kit/main.cpp
	      )
target_link_libraries(
                      measurement_kit
		      geoip
		      crypto
		      ssl
		      ${libevent_static_lib_name}
		      ${osx_extra_libs}
		      ${msvc_extra_libs}
		      measurement_kit_static
		     )
