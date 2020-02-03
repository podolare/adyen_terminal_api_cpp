include(ExternalProject)

ExternalProject_Add(
        jsoncpp
        GIT_REPOSITORY "https://github.com/open-source-parsers/jsoncpp.git"
        UPDATE_COMMAND git pull "https://github.com/open-source-parsers/jsoncpp.git"
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=/usr/local/data -DCMAKE_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp
)