#include <iostream>
#include <iomanip>
#include <chrono>

#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include "../filesystem.h"
namespace fs = ghc::filesystem;
#endif

template<typename TP>
std::time_t to_time_t(TP tp)
{
    // Based on trick from: Nico Josuttis, C++17 - The Complete Guide
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + (tp - TP::clock::now()));
}

static std::string perm_to_str(fs::perms prms)
{
    std::string result;
    result.reserve(6);
    for(int i = 0; i < 9; ++i) {
        result = ((static_cast<int>(prms) & (1<<i)) ? "xwrxwrxwr"[i] : '-') + result;
    }
    return result;
}

int main(int argc, char* argv[])
{
#ifdef GHC_FILESYSTEM_VERSION
    fs::u8arguments u8guard(argc, argv);
#endif
    if(argc > 2) {
        std::cerr << "USAGE: dir <path>" << std::endl;
        exit(1);
    }
    fs::path dir{"."};
    if(argc == 2) {
        dir = fs::u8path(argv[1]);
    }
    for(auto de : fs::directory_iterator(dir)) {
        auto ft = to_time_t(de.last_write_time());
        auto ftm = *std::localtime(&ft);
        std::cout << (de.is_directory() ? "d" : "-") << perm_to_str(de.symlink_status().permissions()) << "  "
                  << std::setw(8) << (de.is_directory() ? "-" : std::to_string(de.file_size())) << "  "
                  << std::put_time(&ftm, "%Y-%m-%d %H:%M:%S") << "  "
                  << de.path().filename().string() 
                  << std::endl;
    }
    return 0;
}
