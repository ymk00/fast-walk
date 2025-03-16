#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <stack>
#include <string>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    unsigned long long file_count{0};
    std::stack<std::string> dirs{};
    dirs.push("/");

    std::string chosen_file{};

    while (!dirs.empty()) {
        std::string cur_dir = dirs.top();
        dirs.pop();

        // https://linux.die.net/man/2/open
        int fd = open(cur_dir.c_str(), O_DIRECTORY | O_RDONLY);
        if (fd == -1) {
            if (errno != EACCES) {
                std::cout << "dir read failed: " << errno << std::endl;
            }
            continue;
        }

        // https://linux.die.net/man/2/getdents64
        char buf[10000];
        for (;;) {
            int nread = syscall(SYS_getdents64, fd, buf, sizeof(buf));
            if (nread == -1) {
                std::cout << errno << std::endl;
                break;
            }
            if (nread == 0) {
                break;
            }

            int off = 0;
            while (off < nread) {
                dirent *ent = reinterpret_cast<dirent *>(buf + off);
                if (std::strcmp(ent->d_name, "..") == 0 ||
                    std::strcmp(ent->d_name, ".") == 0) {
                    off += ent->d_reclen;
                    continue;
                }

                std::string new_path =
                    cur_dir + (cur_dir.back() != '/' ? "/" : "") + ent->d_name;
                if (ent->d_type == DT_DIR) {
                    dirs.push(new_path);
                } else if (ent->d_type == DT_REG) {
                    ++file_count;
                    if (std::rand() % file_count == 0) {
                        chosen_file = new_path;
                    }
                } else if (ent->d_type == DT_LNK) {
                    // symlink, just ignore
                } else if (ent->d_type == DT_BLK || ent->d_type == DT_CHR ||
                           ent->d_type == DT_SOCK || ent->d_type == DT_FIFO) {
                    // devices, socket, named pipe, no chance of handling
                } else if (ent->d_type == DT_UNKNOWN) {
                    std::cout << "unknown file type: " << ent->d_name
                              << std::endl;
                }

                off += ent->d_reclen;
            }
        }

        close(fd);
    }

    std::cout << "file count: " << file_count << '\n';
    std::cout << "chosen file: " << chosen_file << '\n';

    return 0;
}