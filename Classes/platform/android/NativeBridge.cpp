/*
 * NativeBridge.cpp
 *
 *  Created on: 2015/02/17
 *      Author: Chara
 */
#include "platform/NativeBridge.h"
#include "jni/NativeControllerJni.h"
#include "cocos2d.h"

namespace platform
{

void NativeBridge::executeNative()
{
    executeJni();
}

void NativeBridge::executeNative2(int score)
{
    executeJni2(score);
}

std::vector<std::string> NativeBridge::executeNative3()
{
    CCLOG("executeNative3");

    // Jni連携で帰ってくる文字列（カンマ区切り）を配列に詰めなおす。
    std::string scores = executeJni3();

    CCLOG("executeNative3 result: %s", scores.c_str());

    char delim = ',';
    std::vector<std::string> ret;
    ret = split(scores, delim);

    return ret;
}

std::vector<std::string> NativeBridge::split(std::string &str, char delim){
    std::vector<std::string> res;
    size_t current = 0, found;
    while((found = str.find_first_of(delim, current)) != std::string::npos){
        std::string s = std::string(str, current, found - current);
        res.push_back(s);
        current = found + 1;
        CCLOG("split result: %s", s.c_str());
    }
    res.push_back(std::string(str, current, str.size() - current));

    return res;
}

void NativeBridge::executeNative4(float time)
{
    executeJni4(time);
}

std::vector<std::string> NativeBridge::executeNative5()
{
    CCLOG("executeNative5");

    // Jni連携で帰ってくる文字列（カンマ区切り）を配列に詰めなおす。
    std::string times = executeJni5();

    CCLOG("executeNative5 result: %s", times.c_str());

    char delim = ',';
    std::vector<std::string> ret;
    ret = split(times, delim);

    return ret;
}


}


