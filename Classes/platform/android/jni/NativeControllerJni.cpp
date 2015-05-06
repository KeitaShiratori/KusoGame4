/*
 * NativeControllerJni.cpp
 *
 *  Created on: 2015/02/16
 *      Author: Chara
 */
#include "NativeControllerJni.h"
#include "platform/android/jni/JniHelper.h"

extern "C" {
    void executeJni()
    {
        cocos2d::JniMethodInfo t;

        if (cocos2d::JniHelper::getStaticMethodInfo(t, "org.cocos2dx.cpp.AppActivity", "executeJava", "()V")) {
            t.env->CallStaticVoidMethod(t.classID, t.methodID);
            t.env->DeleteLocalRef(t.classID);
        }
    }

    void executeJni2(int score)
    {
        cocos2d::JniMethodInfo t;

        if (cocos2d::JniHelper::getStaticMethodInfo(t,
                "org.cocos2dx.cpp.AppActivity", "executeRegistScore", "(I)V")) {
            t.env->CallStaticVoidMethod(t.classID, t.methodID, score);
            t.env->DeleteLocalRef(t.classID);
        }
    }

    std::string executeJni3()
    {
        CCLOG("executeJni3");
        cocos2d::JniMethodInfo t;
        std::string ret;

        if (cocos2d::JniHelper::getStaticMethodInfo(t,
                "org.cocos2dx.cpp.AppActivity", "executeFindScore", "()Ljava/lang/String;")) {

            jstring jStr = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
            const char* str = t.env->GetStringUTFChars(jStr, NULL);
            ret = str;
            cocos2d::CCLog("executeJni3 ret: %s", ret.c_str() );

            t.env->ReleaseStringUTFChars(jStr,str);
            t.env->DeleteLocalRef(t.classID);
        }

        CCLOG("executeJni3 result: %s", ret.c_str());
        return ret;
    }

    void executeJni4(float time)
    {
        cocos2d::JniMethodInfo t;

        if (cocos2d::JniHelper::getStaticMethodInfo(t,
                "org.cocos2dx.cpp.AppActivity", "executeRegistTime", "(F)V")) {
            t.env->CallStaticVoidMethod(t.classID, t.methodID, time);
            t.env->DeleteLocalRef(t.classID);
        }
    }

    std::string executeJni5()
    {
        CCLOG("executeJni5");
        cocos2d::JniMethodInfo t;
        std::string ret;

        if (cocos2d::JniHelper::getStaticMethodInfo(t,
                "org.cocos2dx.cpp.AppActivity", "executeFindTime", "()Ljava/lang/String;")) {

            jstring jStr = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
            const char* str = t.env->GetStringUTFChars(jStr, NULL);
            ret = str;
            cocos2d::CCLog("executeJni5 ret: %s", ret.c_str() );

            t.env->ReleaseStringUTFChars(jStr,str);
            t.env->DeleteLocalRef(t.classID);
        }

        CCLOG("executeJni5 result: %s", ret.c_str());
        return ret;
    }

}
