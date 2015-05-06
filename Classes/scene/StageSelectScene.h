#ifndef __STAGE_SELECT_SCENE_H__
#define __STAGE_SELECT_SCENE_H__

#include "cocos2d.h"
#include "particlesystem/ParticleSystemPool.h"

USING_NS_CC;

// タイトル表示
class StageSelectScene : public cocos2d::Layer
{
protected:
    enum ZOrder{
        Background = 0,
        Label = 10,
        Icon = 100,
        Dialog
    };

public:
    StageSelectScene(); //コンストラクタ
    virtual bool init(); //初期化
    static StageSelectScene* create(); //create関数生成
    static cocos2d::Scene* createScene(); //シーン生成

    ParticleSystemPool* _pool;//パーティクルプール
    ParticleSystemPool* _clearPool;//パーティクルプール

    //シングルタップイベント
    virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* unused_event);

    void initBackground(); //背景情報の初期化
    void initMenu(); //メニューの初期化

    void nextScene(int NEXT_SCENE, int level, float dt); //画面遷移
};


#endif // __STAGE_SELECT_SCENE_H__
