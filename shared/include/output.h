#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include <mutex>
#include <iostream>
#include <string>

void guarded_cout(const std::string& output, std::mutex& cout_lock);

#endif