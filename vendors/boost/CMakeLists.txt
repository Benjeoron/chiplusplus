include(FetchContent)

set(BOOST_VERSION "1.90.0")

FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.90.0/boost-1.90.0-cmake.7z
    URL_HASH SHA256=218e74c4aa362a994b7b7a23b2920f455a00205c656405fcf262cf60b8871921
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(Boost)