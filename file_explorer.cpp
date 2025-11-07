#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <cstring>
#include <iomanip>
using namespace std;

// Function to display a stylish title banner
void showBanner() {
    cout << "\033[1;36m============================================\n";
    cout << "     🌟 Linux File Explorer Application 🌟   \n";
    cout << "============================================\033[0m\n\n";
}

// Function to list files and directories
void listFiles() {
    cout << "\n\033[1;33m📂 Listing Files in Current Directory:\033[0m\n";
    DIR *dir;
    struct dirent *entry;
    dir = opendir(".");
    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        cout << " - " << entry->d_name << endl;
    }
    closedir(dir);
}

// Function to display current directory path
void showCurrentPath() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    cout << "\n📍 You are currently in: \033[1;32m" << cwd << "\033[0m\n";
}

// Function to change directory
void changeDirectory() {
    string path;
    cout << "\nEnter directory name to navigate: ";
    cin >> path;
    if (chdir(path.c_str()) == 0) {
        cout << "✅ Moved to directory: " << path << endl;
    } else {
        perror("❌ Directory not found");
    }
}

// Function to create a new file
void createFile() {
    string filename;
    cout << "\nEnter filename to create: ";
    cin >> filename;
    ofstream file(filename);
    if (file) {
        cout << "✅ File '" << filename << "' created successfully.\n";
    } else {
        cout << "❌ Error creating file.\n";
    }
}

// Function to delete a file
void deleteFile() {
    string filename;
    cout << "\nEnter filename to delete: ";
    cin >> filename;
    if (remove(filename.c_str()) == 0)
        cout << "🗑️ File '" << filename << "' deleted successfully.\n";
    else
        perror("❌ Error deleting file");
}

// Function to rename or move a file
void renameFile() {
    string oldName, newName;
    cout << "\nEnter existing filename: ";
    cin >> oldName;
    cout << "Enter new filename or path: ";
    cin >> newName;
    if (rename(oldName.c_str(), newName.c_str()) == 0)
        cout << "✅ File renamed/moved successfully.\n";
    else
        perror("❌ Error renaming/moving file");
}

// Recursive function to search for a file
void searchFile(const string &name, const string &path) {
    DIR *dir = opendir(path.c_str());
    if (!dir) return;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name == name)
            cout << "🔍 Found: " << path + "/" + entry->d_name << endl;
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            searchFile(name, path + "/" + entry->d_name);
        }
    }
    closedir(dir);
}

// Function to show file permissions
void showPermissions() {
    string filename;
    cout << "\nEnter filename: ";
    cin >> filename;
    struct stat fileStat;
    if (stat(filename.c_str(), &fileStat) < 0) {
        perror("Error fetching file info");
        return;
    }
    cout << "\n🔐 Permissions for " << filename << ": ";
    cout << ((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    cout << ((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    cout << ((fileStat.st_mode & S_IXUSR) ? "x" : "-");
    cout << ((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    cout << ((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    cout << ((fileStat.st_mode & S_IXGRP) ? "x" : "-");
    cout << ((fileStat.st_mode & S_IROTH) ? "r" : "-");
    cout << ((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    cout << ((fileStat.st_mode & S_IXOTH) ? "x" : "-") << endl;
}

// Function to change file permissions
void changePermissions() {
    string filename;
    int mode;
    cout << "\nEnter filename: ";
    cin >> filename;
    cout << "Enter permission mode (e.g., 777 for full access): ";
    cin >> oct >> mode;
    if (chmod(filename.c_str(), mode) == 0)
        cout << "✅ Permissions updated successfully.\n";
    else
        perror("❌ Failed to change permissions");
}

int main() {
    int choice;
    while (true) {
        showBanner();
        cout << "1️⃣  List Files\n";
        cout << "2️⃣  Show Current Path\n";
        cout << "3️⃣  Change Directory\n";
        cout << "4️⃣  Create New File\n";
        cout << "5️⃣  Delete File\n";
        cout << "6️⃣  Rename / Move File\n";
        cout << "7️⃣  Search File\n";
        cout << "8️⃣  Show File Permissions\n";
        cout << "9️⃣  Change File Permissions\n";
        cout << "0️⃣  Exit\n";
        cout << "\n👉 Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: listFiles(); break;
            case 2: showCurrentPath(); break;
            case 3: changeDirectory(); break;
            case 4: createFile(); break;
            case 5: deleteFile(); break;
            case 6: renameFile(); break;
            case 7: {
                string name;
                cout << "\nEnter filename to search: ";
                cin >> name;
                cout << "\nSearching...\n";
                searchFile(name, ".");
                break;
            }
            case 8: showPermissions(); break;
            case 9: changePermissions(); break;
            case 0: cout << "\n👋 Exiting File Explorer. Goodbye!\n"; return 0;
            default: cout << "❌ Invalid choice. Please try again.\n";
        }

        cout << "\n\033[1;34mPress Enter to continue...\033[0m";
        cin.ignore();
        cin.get();
        system("clear");  // clears screen for a clean look
    }
}
