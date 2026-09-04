cmake_minimum_required(VERSION 3.16)
project(PopWordBot LANGUAGES CXX)
if(MSVC)
    add_compile_options(/utf-8)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Boost REQUIRED COMPONENTS system)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
set(TGBOT_ROOT "C:/Users/TechnoLaptop/tgbot-cpp" CACHE PATH "Path to tgbot-cpp repo root")
add_library(TgBot STATIC IMPORTED)
set_target_properties(TgBot PROPERTIES
    IMPORTED_LOCATION_DEBUG   "${TGBOT_ROOT}/build/Debug/TgBot.lib"
    IMPORTED_LOCATION_RELEASE "${TGBOT_ROOT}/build/Release/TgBot.lib"
    IMPORTED_LOCATION         "${TGBOT_ROOT}/build/Debug/TgBot.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${TGBOT_ROOT}/include"
)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)
add_executable(PopWordBot ${SOURCES})
set_target_properties(PopWordBot PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_compile_definitions(PopWordBot PRIVATE WIN32_LEAN_AND_MEAN)
target_include_directories(PopWordBot PRIVATE
    src
    ${TGBOT_ROOT}/include
)

target_link_libraries(PopWordBot PRIVATE
    TgBot
    Boost::system
    CURL::libcurl
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    SQLite::SQLite3
)

if(WIN32)
    target_link_libraries(PopWordBot PRIVATE ws2_32 wsock32)
endif()
