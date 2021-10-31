#include "output.h"

void guarded_cout(const std::string& output, std::mutex& cout_lock) {
    std::lock_guard<std::mutex> raii(cout_lock);
    std::cout << output << std::endl;
}