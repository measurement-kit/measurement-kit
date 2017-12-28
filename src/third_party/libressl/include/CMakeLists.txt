install(DIRECTORY .
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PATTERN "CMakeLists.txt" EXCLUDE
        PATTERN "compat" EXCLUDE
        PATTERN "Makefile*" EXCLUDE
        # MK: more files we don't want to install
        PATTERN "CMakeFiles" EXCLUDE
        PATTERN "*.cmake" EXCLUDE
        PATTERN "*.vcxproj*" EXCLUDE
        PATTERN "tls.h" EXCLUDE
        PATTERN "pqueue.h" EXCLUDE)
