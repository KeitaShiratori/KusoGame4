#ifndef __TITLE_SCENE_H__
#define __TITLE_SCENE_H__

#include "cocos2d.h"
#include "particlesystem/ParticleSystemPool.h"

USING_NS_CC;

// タイトル表示
class TitleScene : public cocos2d::Layer
{
protected:
    enum ZOrder{
        Background = 0,
        Label,
        Dialog
    };

public:
    TitleScene(); //コンストラクタ
    virtual bool init(); //初期化
    static TitleScene* create(); //create関数生成
    static cocos2d::Scene* createScene(); //シーン生成

    ParticleSystemPool* _pool;//パーティクルプール

    //シングルタップイベント
    virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* unused_event);

    void initBackground(); //背景情報の初期化
    void initTitle(); // タイトルの初期化
    void initMenu(); //メニューの初期化
    void menuCallback(Ref* pSender); //メニューのコールバック
    void menuCallback2(Ref* pSender); //メニューのコールバック

    void nextScene(int NEXT_SCENE, float dt); //画面遷移

    void highScoreCallback(Ref* pSender);
    void showHighScore();
    void menuStartCallback(Ref* Sender);
    void dialogClose();

    void menuAction(Ref* item);

};

#endif // __TITLE_SCENE_H__
