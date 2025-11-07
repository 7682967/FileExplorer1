#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <system_error>
#include <fstream>  


namespace fs = std::filesystem;

// ------------------- Utilities -------------------
std::string perms_to_string(fs::perms p) {
    // Format like rwxr-xr--
    std::string s;
    s += ( (p & fs::perms::owner_read)  != fs::perms::none ) ? 'r' : '-';
    s += ( (p & fs::perms::owner_write) != fs::perms::none ) ? 'w' : '-';
    s += ( (p & fs::perms::owner_exec)  != fs::perms::none ) ? 'x' : '-';
    s += ( (p & fs::perms::group_read)  != fs::perms::none ) ? 'r' : '-';
    s += ( (p & fs::perms::group_write) != fs::perms::none ) ? 'w' : '-';
    s += ( (p & fs::perms::group_exec)  != fs::perms::none ) ? 'x' : '-';
    s += ( (p & fs::perms::others_read)  != fs::perms::none ) ? 'r' : '-';
    s += ( (p & fs::perms::others_write) != fs::perms::none ) ? 'w' : '-';
    s += ( (p & fs::perms::others_exec)  != fs::perms::none ) ? 'x' : '-';
    return s;
}

void print_error(const std::string &msg, const std::error_code &ec) {
    std::cerr << "Error: " << msg;
    if (ec) std::cerr << " (" << ec.message() << ")";
    std::cerr << "\n";
}

// ------------------- Day 1: List directory -------------------
void list_directory(const fs::path &dir) {
    std::error_code ec;
    if (!fs::exists(dir, ec)) {
        print_error("Directory does not exist", ec);
        return;
    }    std::cout << "Listing: " << dir << "\n\n";
    std::cout << std::left << std::setw(40) << "Name"
              << std::setw(10) << "Type"
              << std::setw(12) << "Size"
              << std::setw(12) << "Permissions"
              << "Last Write\n";
    std::cout << std::string(90, '-') << "\n";

    for (auto &entry : fs::directory_iterator(dir, ec)) {
        if (ec) {
            print_error("Directory iteration failed", ec);
            return;
        }
        auto p = entry.path();
        std::string name = p.filename().string();
        std::string type = entry.is_directory() ? "Dir" : "File";
        uintmax_t size = 0;
        if (entry.is_regular_file(ec)) size = entry.file_size(ec);
        auto perms = entry.status(ec).permissions();
        auto ftime = fs::last_write_time(p, ec);
       auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);


        std::cout << std::left << std::setw(40) << name
                  << std::setw(10) << type
                  << std::setw(12) << (entry.is_regular_file() ? std::to_string(size) : "-")
                  << std::setw(12) << perms_to_string(perms)
                  << std::asctime(std::localtime(&cftime));
    }
    std::cout << "\n";
}

// ------------------- Day 2: Navigation -------------------
void show_cwd() {
    std::error_code ec;
    std::cout << "Current directory: " << fs::current_path(ec) << "\n"; if (ec) print_error("Getting current path failed", ec);
}

void change_directory(const std::string &pathStr) {
    std::error_code ec;
    fs::path p = pathStr;
    if (!fs::exists(p, ec) || !fs::is_directory(p, ec)) {
        print_error("Path is not a directory or does not exist", ec);
        return;
    }
    fs::current_path(p, ec);
    if (ec) print_error("Change directory failed", ec);
}

// ------------------- Day 3: File manipulation -------------------
void copy_item(const fs::path &src, const fs::path &dst) {
    std::error_code ec;
    if (!fs::exists(src, ec)) {
        print_error("Source does not exist", ec);
        return;
    }
    if (fs::is_directory(src, ec)) {
        fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
    } else {
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
    }
    if (ec) print_error("Copy failed", ec);
    else std::cout << "Copied " << src << " -> " << dst << "\n";
}

void move_item(const fs::path &src, const fs::path &dst) {
    std::error_code ec;
    fs::rename(src, dst, ec);
    if (ec) {// If rename fails (cross-device), fallback to copy+remove
        copy_item(src, dst);
        fs::remove_all(src, ec);
        if (ec) print_error("Cleanup after move failed", ec);
        else std::cout << "Moved (via copy+remove) " << src << " -> " << dst << "\n";
    } else {
        std::cout << "Moved " << src << " -> " << dst << "\n";
    }
}

void delete_item(const fs::path &target) {
    std::error_code ec;
    if (!fs::exists(target, ec)) {
        print_error("Target does not exist", ec);
        return;
    }
    if (fs::is_directory(target, ec)) {
        fs::remove_all(target, ec);
    } else {
        fs::remove(target, ec);
    }
    if (ec) print_error("Delete failed", ec);
    else std::cout << "Deleted " << target << "\n";
}

void create_file(const fs::path &p) {
    std::error_code ec;
    std::ofstream ofs(p);
    if (!ofs) {
        print_error("Creating file failed", ec);
        return;
    }
    ofs << ""; // empty file
    ofs.close();  std::cout << "Created file: " << p << "\n";
}

void create_directory(const fs::path &p) {
    std::error_code ec;
    if (fs::create_directories(p, ec)) {
        std::cout << "Created directory: " << p << "\n";
    } else {
        print_error("Create directory failed (may already exist)", ec);
    }
}

// ------------------- Day 4: Search -------------------
void search_files(const fs::path &root, const std::string &pattern) {
    std::error_code ec;
    if (!fs::exists(root, ec)) { print_error("Search root does not exist", ec); return; }
    std::cout << "Searching for \"" << pattern << "\" under " << root << " ...\n";
    for (auto &entry : fs::recursive_directory_iterator(root, ec)) {
        if (ec) { print_error("Recursive iteration failed", ec); return; }
        std::string name = entry.path().filename().string();
        if (name.find(pattern) != std::string::npos) {
            std::cout << entry.path() << (entry.is_directory() ? " [dir]" : " [file]") << "\n";
        }
    }
    std::cout << "Search complete.\n";
}

// ------------------- Day 5: Permissions -------------------
void show_permissions(const fs::path &p) {
    std::error_code ec;
    if (!fs::exists(p, ec)) { print_error("Path doesn't exist", ec); return; }
    auto s = fs::status(p, ec);
    if (ec) { print_error("Status failed", ec); return; }   std::cout << p << " : " << perms_to_string(s.permissions()) << "\n";
}

void change_permissions(const fs::path &p, const std::string &modeStr) {
    // modeStr is like "u+rwx,g+rx,o-r"  OR simple octal string like "755"
    std::error_code ec;
    if (!fs::exists(p, ec)) { print_error("Path doesn't exist", ec); return; }

    // Support an octal style like 755
    bool done = false;
    if (modeStr.size() >= 3 && std::all_of(modeStr.begin(), modeStr.end(), [](char c){ return std::isdigit(c); })) {
        // Octal
        int mode = std::stoi(modeStr, nullptr, 8);
        fs::perms newp = fs::perms::none;
        if (mode & 0400) newp |= fs::perms::owner_read;
        if (mode & 0200) newp |= fs::perms::owner_write;
        if (mode & 0100) newp |= fs::perms::owner_exec;
        if (mode & 0040) newp |= fs::perms::group_read;
        if (mode & 0020) newp |= fs::perms::group_write;
        if (mode & 0010) newp |= fs::perms::group_exec;
        if (mode & 0004) newp |= fs::perms::others_read;
        if (mode & 0002) newp |= fs::perms::others_write;
        if (mode & 0001) newp |= fs::perms::others_exec;
        fs::permissions(p, newp, ec);
        done = true;
    }

    if (!done) {
        
        std::cerr << "Complex permission string parsing is limited in this demo. Use octal (e.g., 755) for reliability.\n";
        return;
    }

    if (ec) print_error("Setting permissions failed", ec);
    else std::cout << "Permissions updated for " << p << "\n";
}

// ------------------- Interactive UI -------------------
void print_menu() {
    std::cout << "==== 🗂️ File Explorer Menu ====\n";
    std::cout << std::left; // align left
    std::cout << std::setw(4) << "1️⃣" << " Show Current Path (pwd)\n";
    std::cout << std::setw(4) << "2️⃣" << " List Files (ls)\n";
    std::cout << std::setw(4) << "3️⃣" << " Change Directory (cd)\n";
    std::cout << std::setw(4) << "4️⃣" << " Create New File (touch)\n";
    std::cout << std::setw(4) << "5️⃣" << " Delete File (rm)\n";
    std::cout << std::setw(4) << "6️⃣" << " Rename / Move File (mv)\n";
    std::cout << std::setw(4) << "7️⃣" << " Search File\n";
    std::cout << std::setw(4) << "8️⃣" << " Show File Permissions (perms)\n";
    std::cout << std::setw(4) << "9️⃣" << " Change File Permissions (chmod)\n";
    std::cout << std::setw(4) << "🔟" << " Exit\n"; // 10th option using 🔟 emoji
    std::cout << "\n👉 Enter your choice: ";
}

void cat_file(const fs::path &p) { std::error_code ec;
    if (!fs::exists(p, ec) || !fs::is_regular_file(p, ec)) {
        print_error("File doesn't exist or is not regular", ec);
        return;
    }
    std::ifstream ifs(p);
    if (!ifs) { print_error("Opening file failed", ec); return; }
    std::string line;
    while (std::getline(ifs, line)) {
        std::cout << line << "\n";
    }
}

// parse very simple command line (space separated)
std::vector<std::string> split_cmd(const std::string &line) {
    std::vector<std::string> parts;
    std::string cur;
    bool inQuote = false;
    for (size_t i=0;i<line.size();++i) {
        char c = line[i];
        if (c == '"') inQuote = !inQuote;
        else if (std::isspace((unsigned char)c) && !inQuote) {
            if (!cur.empty()) { parts.push_back(cur); cur.clear(); }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) parts.push_back(cur);
    return parts;
}

int main() {
    std::cout << "Console File Explorer (Linux) — Days 1-5 demo\n";
    show_cwd();
std::string line;
    while (true) {
        print_menu();
        if (!std::getline(std::cin, line)) break;
        auto args = split_cmd(line);
        if (args.empty()) continue;

        std::string cmd = args[0];
        try {
            if (cmd == "0" || cmd == "exit") break;
            else if (cmd == "1" || cmd == "pwd") show_cwd();
            else if (cmd == "2" || cmd == "ls") {
                fs::path p = (args.size() > 1) ? fs::path(args[1]) : fs::current_path();
                list_directory(p);
            }
            else if ((cmd == "3" || cmd == "cd") && args.size() > 1) change_directory(args[1]);
            else if ((cmd == "4" || cmd == "cat") && args.size() > 1) cat_file(args[1]);
            else if ((cmd == "5" || cmd == "copy") && args.size() > 2) copy_item(args[1], args[2]);
            else if ((cmd == "6" || cmd == "move") && args.size() > 2) move_item(args[1], args[2]);
            else if ((cmd == "7" || cmd == "delete") && args.size() > 1) delete_item(args[1]);
            else if ((cmd == "8" || cmd == "touch") && args.size() > 1) create_file(args[1]);
            else if ((cmd == "9" || cmd == "mkdir") && args.size() > 1) create_directory(args[1]);
            else if ((cmd == "10" || cmd == "search") && args.size() > 2) search_files(args[1], args[2]);
            else if ((cmd == "11" || cmd == "perms") && args.size() > 1) show_permissions(args[1]);
            else if ((cmd == "12" || cmd == "chmod") && args.size() > 2) change_permissions(args[1], args[2]);
            else std::cout << "Unknown or malformed command. Try again.\n";
        } catch (const std::exception &ex) {
            std::cerr << "Exception: " << ex.what() << "\n";
        }
    }   std::cout << "Exiting File Explorer. Goodbye.\n";
    return 0;
}
