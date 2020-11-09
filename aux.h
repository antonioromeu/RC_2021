#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include <fstream>
#include <vector>
#include <cstring>

#define BUFSIZE 1024
#define FILENAMESIZE 28
#define DIRSIZE 12
#define GN 32
#define max(A, B) ((A) >= (B) ? (A) : (B))

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ofstream;
using std::ifstream;
using std::swap;
using std::stoi;
using std::to_string;
using std::cin;


bool isNumeric(string str) {
    for (int i = 0; i < (int) str.length(); i++)
        if (!isdigit(str.at(i)))
            return false;
    return true;
}

bool isAlphanumeric(string str) {
    for (int i = 0; i < (int) str.length(); i++)
        if (!isalnum(str.at(i)))
            return false;
    return true;
}

bool checkUID(string str) {
    return (str.length() == 5 && isNumeric(str));
}

bool checkPass(string str) {
    return (str.length() == 8 && isAlphanumeric(str));
}

bool checkFilename(char *str) {
    if (strlen(str) > 24)
        return false;
    for (int i = 0; i < (int) strlen(str); i++) {
        if (isalnum(str[i]) || str[i] == '.' || str[i] == '-' || str[i] == '_')
            continue;
        else
            return false;
    }
    return true;
}

string createString(vector<string> args) {
    string buffer;
    for (int i = 0; i < (int) args.size(); i++)
        buffer += args.at(i);
    return buffer;
}

bool checkDir(string subdir) {
    DIR *d;
    struct dirent *dir;
    d = opendir("./USERS");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (!strcmp(dir->d_name, subdir.c_str())) {
                closedir(d);
                return true;
            }
        }
        closedir(d);
        return false;
    }
    return false;
}

vector<string> createPathFiles(string UID) {
    string base, passFile, regFile, loginFile, tidFile;
    vector<string> matrix;
    base.clear();
    passFile.clear();
    regFile.clear();
    loginFile.clear();
    tidFile.clear();
    base += "USERS/" + UID + "/";
    passFile += base + UID + "_pass.txt";
    regFile += base + UID + "_reg.txt";
    loginFile += base + UID + "_login.txt";
    tidFile += base + UID + "_tid.txt";
    matrix.push_back(passFile);
    matrix.push_back(regFile);
    matrix.push_back(loginFile);
    matrix.push_back(tidFile);
    return matrix;
}