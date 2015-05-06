/*
 * NativeBridge.h
 *
 *  Created on: 2015/02/17
 *      Author: Chara
 */

#ifndef __NATIVEBRIDGE_H_
#define __NATIVEBRIDGE_H_

#include "cocos2d.h"

namespace platform
{

class NativeBridge
{
public:
    static void executeNative();
    static void executeNative2(int score);
    static std::vector<std::string> executeNative3();
    static void executeNative4(float time);
    static std::vector<std::string> executeNative5();

private:
    static std::vector<std::string> split(std::string &scores, char delim);

};

}


#endif /* __NATIVEBRIDGE_H_ */
