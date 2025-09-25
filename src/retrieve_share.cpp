#include <string>
#include <filesystem> // C++17

std::filesystem::path getExecutablePath() {
    // Platform-specific code to get the path of the running executable
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return path;
#elif __APPLE__
    char path[PATH_MAX];
    uint32_t size = PATH_MAX;
    _NSGetExecutablePath(path, &size);
    return path;
#else // Linux and others
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        return path;
    }
    return "";
#endif
}

int retrieve() {
    std::filesystem::path exe_path = getExecutablePath();
    std::filesystem::path exe_dir = exe_path.parent_path();
    
    // Construct the path to the data directory
    std::filesystem::path data_dir = exe_dir.parent_path() / "share" / "my_project_name" / "data";
    
    // Example: Construct the full path to a config file
    std::filesystem::path config_path = data_dir / "config.json";
    
    // Now you can use config_path to load your file
    // ...
    
    return 0;
}