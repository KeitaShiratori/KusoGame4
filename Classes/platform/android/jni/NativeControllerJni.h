/*
 * NativeController.h
 *
 *  Created on: 2015/02/16
 *      Author: Chara
 */

#ifndef __NATIVECONTROLLER_H_
#define __NATIVECONTROLLER_H_

#include "cocos2d.h"

extern "C" {
    extern void executeJni();
    extern void executeJni2(int score);
    extern std::string executeJni3();
    extern void executeJni4(float time);
    extern std::string executeJni5();
}



#endif /* NATIVECONTROLLER_H_ */
