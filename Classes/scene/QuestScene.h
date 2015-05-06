#ifndef __QUEST_SCENE_H__
#define __QUEST_SCENE_H__

#include "cocos2d.h"
#include "sprite/EnemySprite.h"
#include "particlesystem/ParticleSystemPool.h"

using namespace cocos2d;

// オラオラカウンター
class QuestScene : public cocos2d::Layer
{
protected:
    enum ZOrder{
        Background = -1,
        Enemy      = 0,
        Effect     = 200,
        Label,
        Level,
        Dialog,
        Anime
    };

    enum GameStatus{
        Ready = 0,
        Playing,
        Result,
        Failed
    };

    int _level; //現在のレベル
    int _nextLevel; //次のレベル
    bool _touchable; //画面のタッチを受け付けるフラグ
    bool _pFlg; // 次に置くメダルがpTypeかどうかのフラグ
    bool _changeScene; //画面遷移中フラグ
    int _putMedalNum; // 配置されたメダルの数
    int _numberOfTouch; //タッチしている指の本数
    Vec2 _firstTouchPos; // 最初にタッチした座標
    Sprite* _pullVector; // 引っ張った時の矢印画像
    ParticleSystemPool* _pool;//パーティクルプール
    ParticleSystemPool* _touchPool;//パーティクルプール

    CC_SYNTHESIZE(GameStatus, _state, State); //ゲームのステータス管理
    CC_SYNTHESIZE_RETAIN(cocos2d::Label *, _levelLabel, LevelLabel); //ゲームの制限時間ラベル

    Vector<EnemySprite*> _vMedal; //敵
    Vector<CCSpriteBatchNode*> _vMedalNode; //敵画像生成用バッチノード
    Vector<Sprite*> _vBackground;

public:
    QuestScene(); //コンストラクタ
    virtual bool init(int level, float totalTime); //初期化
    static QuestScene* create(int level, float totalTime); //create関数生成
    static cocos2d::Scene* createScene(int level = 1, float totalTime = 0.0f); //シーン生成

    //シングルタップイベント
    virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* unused_event);

    void touchEffect(Touch* touch); //タッチした場所にエフェクト表示
    void touchEffect(Vec2 pos); //タッチした場所にエフェクト表示
    void removeAllAnimation(Node* sender);
    void initBackground(); //背景情報の初期化
    void initLevel(); //レベルの初期化
    void initTimer(); //タイマーの初期化
    void initBattleField(); //フィールドの初期化
    void initMedalNum(); //残り敵数の初期化
    void initMedal(); //敵情報の初期化
    void initMenu(); //メニューの初期化
    bool isEmptySpace(Vec2 pos, Vector<EnemySprite*> vMedal);
    void putMedal(Touch* touch);
    void putPMedal(Touch* touch);
    void putMyPMedal(Touch* touch);
    void putNMedal(Touch* touch);
    void putMedal(Touch* touch, bool pFlg);
    void putMedal(Vec2 pos, bool pFlg);
    void putMedal(Vec2 pos, bool pFlg, int charge, int mass);
    void putMedal(Vec2 pos, bool pFlg, bool myFlg);
    void putMedal(Vec2 pos, bool pFlg, bool myFlg, int charge, int mass);
    void drawRectangle(CCPoint* points[]); //四角形の描画
    cocos2d::Sprite* createEnemy(Texture2D* texture); // 敵の生成
    void layoutMedal(); // 敵を配置する。
    void fieldColor();
    void initLevelLayer(); //レベルレイヤーの生成
    void removeLevelLayer(float dt); //レベルレイヤーの削除
    void initTutorial(); // チュートリアル用のメッセージを出力
    void showHint(std::string title);
    void showHint(std::string title, bool actionFlg);
    std::string getText();
    void menuStartCallback(Ref* Sender);
    void dialogClose();

    void update(float dt);
    bool isOutOfBattleField(EnemySprite* medal); // メダルが場外に出たか判定
    bool isOutOfBattleField(Vec2 pos); // 座標が場外に出たか判定
    void reflectVelocity(EnemySprite* medal);
    void destroyEffect(EnemySprite* medal); // メダルを破壊するエフェクト
    bool isStageClear(); //ステージクリア判定
    bool isStageClearInLoop();
    bool isStageFailed();
    bool isStageFailedInLoop();

    void resultAnimation();
    void failedAnimation();
    void addColorLayer(Color4B color);
    void showSpriteAndSlideInTop(std::string filepath);

    void nextScene(float dt);

    void adviceScene(float dt);

    void menuCallback(Ref* pSender);
    void backTitle(float dt);

};

#endif // __QUEST_SCENE_H__
