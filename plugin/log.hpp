#ifndef __LOG_H
#define __LOG_H

#include <pro.h>
#include <kernwin.hpp>
#include "loguru.hpp"

#define VA_ARGS(...) , ##__VA_ARGS__
#define LOG(message,...) msg("[*] " message "\n" VA_ARGS(__VA_ARGS__)); LOG_F(INFO, message VA_ARGS(__VA_ARGS__))
#define WARN(message,...) msg("[!] " message "\n" VA_ARGS(__VA_ARGS__)); LOG_F(WARNING, message VA_ARGS(__VA_ARGS__))
#define ERR(message,...) msg("[!] " message "\n" VA_ARGS(__VA_ARGS__)); LOG_F(ERROR, message VA_ARGS(__VA_ARGS__))

#endif /* __LOG_H */
