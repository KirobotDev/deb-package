#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/utsname.h>
#include <limits.h>
#include <pwd.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iomanip>
#include <sstream>
#include <memory>
#include <array>
#include <cmath>
#include <algorithm>

using namespace std;

string exec(const char* cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "Unknown";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    // Remove trailing newline
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result.empty() ? "0" : result;
}

string get_os_name() {
    ifstream file("/etc/os-release");
    string line;
    while (getline(file, line)) {
        if (line.find("PRETTY_NAME=") == 0) {
            string name = line.substr(12);
            if (name.front() == '"') name = name.substr(1, name.length() - 2);
            return name;
        }
    }
    return "Linux";
}

string get_uptime() {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        long uptime = info.uptime;
        long days = uptime / 86400;
        long hours = (uptime % 86400) / 3600;
        long minutes = (uptime % 3600) / 60;
        
        stringstream ss;
        if (days > 0) ss << days << " days, ";
        ss << hours << " hours, " << minutes << " mins";
        return ss.str();
    }
    return "Unknown";
}

string get_shell() {
    char* shell = getenv("SHELL");
    if (!shell) return "Unknown";
    string s(shell);
    string ver = exec((s + " --version | head -n1 | awk '{print $4}'").c_str());
    return s.substr(s.find_last_of('/') + 1) + " " + ver;
}

string get_cpu() {
    ifstream file("/proc/cpuinfo");
    string line;
    string model;
    int cores = 0;
    while (getline(file, line)) {
        if (line.find("model name") != string::npos) {
            if (model.empty()) {
                size_t pos = line.find(":");
                if (pos != string::npos) model = line.substr(pos + 2);
            }
            cores++;
        }
    }
    string freq = exec("lscpu | grep 'CPU max MHz' | awk '{print $4}'");
    if (freq == "0" || freq == "") freq = exec("cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    
    double freq_ghz = 0.0;
    try {
         freq_ghz = stod(freq) / 1000.0;
         if (freq_ghz > 100) freq_ghz /= 1000.0; 
    } catch (...) {}

    stringstream ss;
    ss << model << " (" << cores << ")";
    if (freq_ghz > 0) ss << " @ " << fixed << setprecision(2) << freq_ghz << " GHz";
    return ss.str();
}

string get_gpu() {
    string gpu = exec("lspci | grep -i 'vga\\|3d' | cut -d ':' -f3 | head -n1");
    size_t first = gpu.find_first_not_of(' ');
    if (string::npos == first) return gpu;
    return gpu.substr(first);
}

string get_memory() {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        long total = info.totalram * info.mem_unit;
        long free = info.freeram * info.mem_unit;
        
        string used_s = exec("free -m | grep Mem | awk '{print $3}'");
        string total_s = exec("free -m | grep Mem | awk '{print $2}'");
        
        double used_gi = stod(used_s) / 1024.0;
        double total_gi = stod(total_s) / 1024.0;
        double pct = (used_gi / total_gi) * 100.0;
        
        stringstream ss;
        ss << fixed << setprecision(2) << used_gi << " GiB / " << total_gi << " GiB (" << (int)pct << "%)";
        return ss.str();
    }
    return "Unknown";
}

string get_disk() {
    struct statvfs buffer;
    if (statvfs("/", &buffer) == 0) {
        double total = (double)buffer.f_blocks * buffer.f_frsize;
        double available = (double)buffer.f_bavail * buffer.f_frsize;
        double used = total - available;
        
        double total_gi = total / (1024*1024*1024);
        double used_gi = used / (1024*1024*1024);
        double pct = (used_gi / total_gi) * 100.0;
        
        stringstream ss;
        ss << fixed << setprecision(2) << used_gi << " GiB / " << total_gi << " GiB (" << (int)pct << "%) - ext4"; // Assuming ext4 for simplicity or parse mount
        return ss.str();
    }
    return "Unknown";
}

string get_local_ip() {
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;
    string ip = "Unknown";
    string iface = "";

    getifaddrs(&ifAddrStruct);
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            string name = ifa->ifa_name;
            if (name != "lo") {
                 ip = addressBuffer;
                 iface = name;
                 break; 
            }
        }
    }
    if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
    if (!iface.empty()) return ip + " (" + iface + ")";
    return ip;
}

string get_packages() {
    string dpkg = exec("dpkg-query -f '${binary:Package}\\n' -W | wc -l");
    string flatpak = exec("flatpak list --app | wc -l");
    string snap = exec("snap list | wc -l");
    return dpkg + " (dpkg), " + flatpak + " (flatpak), " + snap + " (snap)";
}

void show_fetch() {
    vector<string> ascii_lines;
    ifstream ascii_file("/usr/share/fks/ascii.txt");
    if (ascii_file.is_open()) {
        string line;
        while (getline(ascii_file, line)) ascii_lines.push_back(line);
        ascii_file.close();
    } else {
        ascii_lines.push_back("(ASCII art not found)");
    }

    struct utsname buffer;
    uname(&buffer);
    
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    string user = getenv("USER") ? getenv("USER") : "user";
    string de = getenv("XDG_CURRENT_DESKTOP") ? getenv("XDG_CURRENT_DESKTOP") : "Unknown";
    string wm = getenv("XDG_SESSION_DESKTOP") ? getenv("XDG_SESSION_DESKTOP") : "Unknown";
    string term = getenv("TERM") ? getenv("TERM") : "Unknown";

    vector<string> info;
    info.push_back("\033[1;32m" + user + "@" + string(hostname) + "\033[0m");
    info.push_back("---------------------------------");
    info.push_back("\033[1;34mOS\033[0m: " + get_os_name());
    info.push_back("\033[1;34mKernel\033[0m: " + string(buffer.sysname) + " " + string(buffer.release));
    info.push_back("\033[1;34mUptime\033[0m: " + get_uptime());
    info.push_back("\033[1;34mPackages\033[0m: " + get_packages());
    info.push_back("\033[1;34mShell\033[0m: " + get_shell());
    info.push_back("\033[1;34mDE\033[0m: " + de);
    info.push_back("\033[1;34mWM\033[0m: " + wm);
    info.push_back("\033[1;34mTerminal\033[0m: " + term);
    info.push_back("\033[1;34mCPU\033[0m: " + get_cpu());
    info.push_back("\033[1;34mGPU\033[0m: " + get_gpu());
    info.push_back("\033[1;34mMemory\033[0m: " + get_memory());
    info.push_back("\033[1;34mDisk (/)\033[0m: " + get_disk());
    info.push_back("\033[1;34mLocal IP\033[0m: " + get_local_ip());
    info.push_back("\033[1;34mLocale\033[0m: " + (getenv("LANG") ? string(getenv("LANG")) : "Unknown"));

    size_t max_lines = max(ascii_lines.size(), info.size());
    for (size_t i = 0; i < max_lines; ++i) {
        if (i < ascii_lines.size()) {
             cout << ascii_lines[i];
             cout << "\t"; 
        } else {
             cout << "\t\t\t\t\t\t"; 
        }
        
        if (i < info.size()) {
            cout << info[i];
        }
        cout << endl;
    }
    cout << endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        
        vector<char*> args;
        args.push_back((char*)"nano");
        args.push_back((char*)"-l"); 
        args.push_back((char*)"-i"); 
        args.push_back((char*)"-c"); 
        args.push_back((char*)"-m"); 
        
        for (int i = 1; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        args.push_back(nullptr);
        
        execvp("nano", args.data());
        perror("Failed to start nano");
        return 1;
    } else {
        show_fetch();
    }
    return 0;
}
