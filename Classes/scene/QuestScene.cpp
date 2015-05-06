#include "QuestScene.h"
#include "scene/StageSelectScene.h"
#include "platform/NativeBridge.h"
#include "stdlib.h"
#include "utility/UIDialog.h"

#include "extensions/cocos-ext.h"

USING_NS_CC_EXT;
USING_NS_CC;

#define WINSIZE Director::getInstance()->getWinSize()
#define TAG_LEVEL_LAYER 10000
#define TAG_ENEMY_NODE  20000
#define TAG_ENEMY_NODE1 20001
#define TAG_ENEMY_NODE_PLUS  20001
#define TAG_ENEMY_NODE_MINUS 20002
#define TAG_ENEMY_NODE3 20003
#define TAG_ENEMY_NODE4 20004
#define TAG_ENEMY_NODE5 20005
#define TAG_ENEMY_NODE6 20006
#define FONT_SIZE 64
#define MAX_LEVEL 50
#define MAX_PULL_LENGTH 200.0f
#define BEST_SCORE "BEST_SCORE"
#define T_DIALOG 1000

//コンストラクタ
QuestScene::QuestScene() :
        _level(0),
        _nextLevel(0),
        _putMedalNum(0),
        _numberOfTouch(0),
        _firstTouchPos(Vec2::ZERO),
        _state(GameStatus::Ready),
        _touchable(false),
        _changeScene(false),
        _pFlg(true),
        _levelLabel(NULL)
{
    // メダル配列の初期化
    _vMedal = Vector<EnemySprite*>(0);
    // メダル画像配列の初期化
    _vMedalNode = Vector<CCSpriteBatchNode*>(0);
    _vBackground = Vector<Sprite*>(0);
}

//シーン生成
Scene* QuestScene::createScene(int level, float totalTime) {
    auto scene = Scene::create();
    auto layer = QuestScene::create(level, totalTime);
    scene->addChild(layer);
    return scene;
}

//インスタンス生成
QuestScene* QuestScene::create(int level, float totalTime) {
    QuestScene *pRet = new QuestScene();
    pRet->init(level, totalTime);
    pRet->autorelease();

    return pRet;
}

//初期化
bool QuestScene::init(int level, float totalTime) {
    if (!Layer::init())
        return false;

    // シングルタップイベントの取得
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(_swallowsTouches);
    touchListener->onTouchBegan = CC_CALLBACK_2(QuestScene::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(QuestScene::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(QuestScene::onTouchEnded, this);
    touchListener->onTouchCancelled = CC_CALLBACK_2(QuestScene::onTouchCancelled, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    _level = level; //レベルの保持

    // プールを生成してメンバー変数に持っておく
    _touchPool = ParticleSystemPool::create("touch_texture.plist", 50);
    _touchPool->retain();
    _pool = ParticleSystemPool::create("destroy_texture.plist", 50);
    _pool->retain();

    initBackground(); //背景の初期化
    initLevel(); //レベルの初期化
    initBattleField(); //フィールドの初期化
    initMedal(); // メダルの初期化
    initMenu(); // メニューの初期化
    initLevelLayer(); //レベル表示レイヤーの表示
    initTutorial();

    // updateを毎フレーム実行するように登録する。
    this->scheduleUpdate();
    return true;
}

bool QuestScene::onTouchBegan(Touch* touch, Event* unused_event) {
    CCLOG("onToucnBegan");

    touchEffect(touch);

    if (!_touchable) {
        return false;
    }

    // すでにタッチしている指が存在したら処理中断する。
    if (_numberOfTouch > 0) {
        return false;
    }

    if(isOutOfBattleField(touch->getLocation())){
        return false;
    }

    // すでに配置されているメダルと重なっているときは置けない。
    if(!isEmptySpace(touch->getLocation(), _vMedal)){
        return false;
    }

    putMyPMedal(touch);

    fieldColor();

    // 各メダルの移動方向を表示
    for(auto medal : _vMedal){
        if(medal->isDead()){
            continue;
        }
        if(medal->isMyMedal()){
            continue;
        }
        medal->calculateVelocity(_vMedal);
        medal->setVelocity(Vec2::ZERO);
        medal->setAbsVelocity(0.0f);
    }

    // タッチしている指の本数をインクリメント
    _numberOfTouch++;

    // タッチした位置を保持しておく
    _firstTouchPos = touch->getLocation();

    return true;
}

bool QuestScene::isEmptySpace(Vec2 pos, Vector<EnemySprite*> vMedal){
    bool ret = true;
    for(auto medal : vMedal){
        if(medal->isTouchPoint(pos)){
            ret = false;
        }
    }
    return ret;
}

void QuestScene::onTouchMoved(Touch* touch, Event* unused_event) {
    CCLOG("onTouchMoved");

    touchEffect(_firstTouchPos);

    // タッチした座標がバトルフィールドの外側の場合、座標を修正する。
    auto touchPos = touch->getLocation();
    if(touchPos.x < (WINSIZE.width * 0.1)){
        touchPos.x = WINSIZE.width * 0.1;
    }
    if(touchPos.x > (WINSIZE.width * 0.9)){
        touchPos.x = WINSIZE.width * 0.9;
    }
    if(touchPos.y < (WINSIZE.height * 0.1)){
        touchPos.y = WINSIZE.height * 0.1;
    }
    if(touchPos.y > (WINSIZE.height * 0.9)){
        touchPos.y = WINSIZE.height * 0.9;
    }

    // 現在のタッチ座標と初期タッチ座標の差分を求める。
    auto pullVec2 = touchPos - _firstTouchPos;
    float len = sqrt(pullVec2.x * pullVec2.x + pullVec2.y * pullVec2.y);

    // 最初の座標と同じ場所をタッチしていたら処理終了
    if(len == 0){
        return;
    }

    auto pullVec2Unit = pullVec2 / len;

    // 引っ張った方向にメダルの座標を変更する。（引っ張り距離の最大値より大きくは引っ張れない。）
    auto medal = _vMedal.at(_vMedal.size() - 1);

    if(len > MAX_PULL_LENGTH){
        len = MAX_PULL_LENGTH;
    }

    _pullVector->setPosition(_firstTouchPos);
    _pullVector->setScale(0.3 + 0.7 * len / MAX_PULL_LENGTH);
    float angle = ccpToAngle(pullVec2Unit)*-180/M_PI;
    _pullVector->setRotation(angle - 90.0f);

    medal->setPosition(_firstTouchPos + len * pullVec2Unit);

    // 引っ張りに応じて、メダルにvelocityとabsVelocityをセットする。
    medal->setVelocity(-len * pullVec2Unit * 8000);
    medal->setAbsVelocity(len);

    // 各メダルの移動方向を表示
    for(auto m : _vMedal){
        if(m->isDead()){
            continue;
        }
        if(m->isMyMedal()){
            continue;
        }
        m->calculateVelocity(_vMedal);
        m->setVelocity(Vec2::ZERO);
        m->setAbsVelocity(0.0f);
    }
}

void QuestScene::onTouchEnded(Touch* touch, Event* unused_event) {
    CCLOG("onTouchEnded");

    _touchable = false;

    _numberOfTouch--;

    _pullVector->setPosition(ccp(-1000, -1000));
}

void QuestScene::onTouchCancelled(Touch* touch, Event* unused_event) {
    CCLOG("onTouchCancelled");
    _numberOfTouch--;
}

void QuestScene::touchEffect(Touch* touch){
    touchEffect(touch->getLocation());
}

void QuestScene::touchEffect(Vec2 pos){
    auto touchP = _touchPool->pop();
    if(touchP != nullptr){
        touchP->setPosition(pos);
        touchP->setAutoRemoveOnFinish(true); // 表示が終わったら自分を親から削除！
        this->addChild(touchP, Effect);
    }
}

void QuestScene::putMedal(Touch* touch){
    putMedal(touch, _pFlg);
}

void QuestScene::putMyPMedal(Touch* touch){
    bool pFlg = true;
    bool myFlg = true;
    putMedal(touch->getLocation(), pFlg, myFlg);
}

void QuestScene::putPMedal(Touch* touch){
    bool pFlg = true;
    putMedal(touch, pFlg);
}

void QuestScene::putNMedal(Touch* touch){
    bool pFlg = false;
    putMedal(touch, pFlg);
}

void QuestScene::putMedal(Touch* touch, bool pFlg){
    putMedal(touch->getLocation(), pFlg);
}

void QuestScene::putMedal(Vec2 pos, bool pFlg){
    bool myFlg = false;
    putMedal(pos, pFlg, myFlg);
}

void QuestScene::putMedal(Vec2 pos, bool pFlg, int charge, int mass){
    bool myFlg = false;
    putMedal(pos, pFlg, myFlg, charge, mass);
}

void QuestScene::putMedal(Vec2 pos, bool pFlg, bool myFlg){
    putMedal(pos, pFlg, myFlg, 1, 1);
}

void QuestScene::putMedal(Vec2 pos, bool pFlg, bool myFlg, int charge, int mass){
    CCLOG("putMedal");

    // タッチした場所にメダルを配置する
    CCSpriteBatchNode* node;
    if(pFlg){
        node = _vMedalNode.at(0);
    }else{
        node = _vMedalNode.at(1);
    }
    auto medal = EnemySprite::createWithTexture(node->getTexture());

    medal->setPosition(pos);
    medal->isMyMedal(myFlg);
    medal->setCharge(charge);
    medal->setMass(mass);
    medal->runAction(RepeatForever::create(
                        Sequence::create(
                                FadeTo::create(0.5f, 208),
                                FadeTo::create(0.5f, 244),
                                nullptr)));

    if(pFlg){
        medal->setPType();
    }else {
        medal->setNType();
    }

    auto light = Sprite::create("ballLight.png");
    light->setPosition(medal->getContentSize() / 2);
    light->setOpacity(255);
    light->setScale(0.42f);
    light->runAction(RepeatForever::create(
                        Sequence::create(
                                FadeTo::create(0.5f, 56),
                                FadeTo::create(0.5f, 255),
                                nullptr)));
    medal->addChild(light, -1);

    medal->describe();

    _vMedal.pushBack((EnemySprite*) medal);
    this->addChild(medal, _vMedal.size());
}

//背景の初期化
void QuestScene::initBackground() {
    CCLOG("initBackground");

    // 引っ張った時の矢印画像
    _pullVector = Sprite::create("vector.png");
    _pullVector->setPosition(Vec2(-1000, -1000));

    _pullVector->runAction(RepeatForever::create(
                        Sequence::create(
                                FadeTo::create(1.0f, 172),
                                FadeTo::create(1.0f, 244),
                                nullptr)));

    this->addChild(_pullVector, Effect);

}

// フィールドの初期化
void QuestScene::initBattleField(){
    CCLOG("initBattleField");

    // 戦場を四角で埋め尽くす。
    int repeatNumX   = 16;
    int repeatNumY   = 24;
    float startWidth  = WINSIZE.width * 0.1f;
    float endWidth    = WINSIZE.width * 0.9f;
    float unitWidth   = (endWidth - startWidth) / repeatNumX;
    float startHeight = WINSIZE.height * 0.1f;
    float endHeight   = WINSIZE.height * 0.9f;
    float unitHeight  = (endHeight - startHeight) / repeatNumY;

    for(int x = 1; x <= repeatNumX; x++){
        for(int y = 1; y <=
        repeatNumY; y++){
            Sprite* sprite = Sprite::create();
            sprite->setTextureRect(Rect(unitWidth * -3, unitHeight * -3, unitWidth * 2, unitHeight * 2));
            sprite->setColor(Color3B(8, 8, 8));
            sprite->setOpacity(56);

            float w = startWidth + unitWidth * x;
            float h = startHeight + unitHeight * y;
            sprite->setPosition(Vec2(w, h));
            sprite->setAnchorPoint(Vec2(0.5f, 0.5f));
            this->addChild(sprite, -1);

            _vBackground.pushBack(sprite);
        }
    }
    // 描画用ノードの作成
    CCDrawNode* draw = CCDrawNode::create();
    draw->setPosition(ccp(0, 0));
    draw->setTag(1);
    this->addChild(draw);

    int WIDTH = 35;
    /* 多角形の描画 */
    static CCPoint points1[] = {
        ccp(WINSIZE.width * 0.1        , WINSIZE.height * 0.1),
        ccp(WINSIZE.width * 0.9        , WINSIZE.height * 0.1),
        ccp(WINSIZE.width * 0.9        , WINSIZE.height * 0.1 - WIDTH),
        ccp(WINSIZE.width * 0.1        , WINSIZE.height * 0.1 - WIDTH),
    };
    static CCPoint points2[] = {
        ccp(WINSIZE.width * 0.9        , WINSIZE.height * 0.1 - WIDTH),
        ccp(WINSIZE.width * 0.9 + WIDTH, WINSIZE.height * 0.1 - WIDTH),
        ccp(WINSIZE.width * 0.9 + WIDTH, WINSIZE.height * 0.9 + WIDTH),
        ccp(WINSIZE.width * 0.9        , WINSIZE.height * 0.9 + WIDTH),
    };
    static CCPoint points3[] = {
        ccp(WINSIZE.width * 0.9        , WINSIZE.height * 0.9),
        ccp(WINSIZE.width * 0.9        , WINSIZE.height * 0.9 + WIDTH),
        ccp(WINSIZE.width * 0.1        , WINSIZE.height * 0.9 + WIDTH),
        ccp(WINSIZE.width * 0.1        , WINSIZE.height * 0.9),
    };
    static CCPoint points4[] = {
        ccp(WINSIZE.width * 0.1        , WINSIZE.height * 0.9 + WIDTH),
        ccp(WINSIZE.width * 0.1 - WIDTH, WINSIZE.height * 0.9 + WIDTH),
        ccp(WINSIZE.width * 0.1 - WIDTH, WINSIZE.height * 0.1 - WIDTH),
        ccp(WINSIZE.width * 0.1        , WINSIZE.height * 0.1 - WIDTH),
    };
    draw->drawPolygon(points1,                  // 頂点の座標のデータ
                      4,                       // 角数
                      ccc4FFromccc3B(ccGREEN), // 図形の色
                      10,                      // 枠線の太さ
                      ccc4FFromccc3B(ccWHITE)  // 枠線の色
                      );
    draw->drawPolygon(points2,                  // 頂点の座標のデータ
                      4,                       // 角数
                      ccc4FFromccc3B(ccGREEN), // 図形の色
                      10,                      // 枠線の太さ
                      ccc4FFromccc3B(ccWHITE)  // 枠線の色
                      );
    draw->drawPolygon(points3,                  // 頂点の座標のデータ
                      4,                       // 角数
                      ccc4FFromccc3B(ccGREEN), // 図形の色
                      10,                      // 枠線の太さ
                      ccc4FFromccc3B(ccWHITE)  // 枠線の色
                      );
    draw->drawPolygon(points4,                  // 頂点の座標のデータ
                      4,                       // 角数
                      ccc4FFromccc3B(ccGREEN), // 図形の色
                      10,                      // 枠線の太さ
                      ccc4FFromccc3B(ccWHITE)  // 枠線の色
                      );
}

//レベルの初期化
void QuestScene::initLevel() {
    // レベルヘッダーの追加
    auto levelLabelHeader = Label::createWithSystemFont("LEVEL", "fonts/Marker Felt.ttf", FONT_SIZE);

    levelLabelHeader->enableShadow(Color4B::WHITE, Size(0.5, 0.5), 15);
    levelLabelHeader->enableOutline(Color4B::WHITE, 5);
    levelLabelHeader->setPosition(Vec2(WINSIZE.width * 0.2, WINSIZE.height - 100));

    this->addChild(levelLabelHeader, Label);

    // レベルラベルの追加
    auto levelLabel = Label::createWithSystemFont(StringUtils::toString(_level), "fonts/Marker Felt.ttf", FONT_SIZE);
    this->setLevelLabel(levelLabel);

    levelLabel->enableShadow(Color4B::WHITE, Size(0.5, 0.5), 15);
    levelLabel->enableOutline(Color4B::WHITE, 5);
    levelLabel->setPosition(Vec2(WINSIZE.width * 0.35, WINSIZE.height - 100));

    this->addChild(levelLabel, Label);
}

//残り敵数の初期化
void QuestScene::initMedalNum() {
    CCLOG("initMedalNum");

}

void QuestScene::initMedal() {
    CCLOG("initMedal");

    for(int i = 1; i <= 2; i++){
        auto medalNode = CCSpriteBatchNode::create(StringUtils::format("ball%d.png", i));
        _vMedalNode.pushBack(medalNode);
        CCLOG("initMedal._vMedalNode.size(): %i", _vMedalNode.size());
        this->addChild(medalNode, Enemy, TAG_ENEMY_NODE + i);
    }

    layoutMedal();

    fieldColor();

    // 各メダルの移動方向を表示
    for(auto medal : _vMedal){
        if(medal->isDead()){
            continue;
        }
        medal->calculateVelocity(_vMedal);
        medal->setVelocity(Vec2::ZERO);
        medal->setAbsVelocity(0.0f);
    }

}

void QuestScene::layoutMedal(){
    switch(_level){
        case 1:
            // P:0 N:1
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.55), false);
            break;
        case 2:
            // P:0 N:1
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.65), false);
            break;
        case 3:
            // P:0 N:5 エックス型
            putMedal(Vec2(WINSIZE.width * 0.35, WINSIZE.height * 0.65), false);
            putMedal(Vec2(WINSIZE.width * 0.35, WINSIZE.height * 0.55), false);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.60), false);
            putMedal(Vec2(WINSIZE.width * 0.65, WINSIZE.height * 0.55), false);
            putMedal(Vec2(WINSIZE.width * 0.65, WINSIZE.height * 0.65), false);
            break;
        case 4:
            // P:6 N:1 花型
            putMedal(Vec2(WINSIZE.width * 0.35, WINSIZE.height * 0.65), true);
            putMedal(Vec2(WINSIZE.width * 0.35, WINSIZE.height * 0.55), true);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.70), true);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.60), false);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.50), true);
            putMedal(Vec2(WINSIZE.width * 0.65, WINSIZE.height * 0.55), true);
            putMedal(Vec2(WINSIZE.width * 0.65, WINSIZE.height * 0.65), true);
            break;
        case 5:
            // P:0 N:2 対角線
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.80), false, 3, 1);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.20), false, 1, 3);
            break;
        case 6:
            // P:0 N:3 正三角形
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.55), false);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.65), false);
            putMedal(Vec2(WINSIZE.width * 0.60, WINSIZE.height * 0.55), false);
            break;
        case 7:
            // P:3 N:1 ひし形
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.75), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.55), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.65), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.65), true , 1, 1);
            break;
        case 8:
            // P:2 N:1 倒立の正三角形
            putMedal(Vec2(WINSIZE.width * 0.33, WINSIZE.height * 0.85), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.65), false, 2, 1);
            putMedal(Vec2(WINSIZE.width * 0.67, WINSIZE.height * 0.85), true , 1, 1);
            break;
        case 9:
            // P:0 N:3 倒立の二等辺三角形（細長いチーズケーキ）
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.75), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.25), false, 1, 2);
            putMedal(Vec2(WINSIZE.width * 0.60, WINSIZE.height * 0.75), false, 1, 1);
            break;
        case 10:
            // P:1 N:3 ひし形
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.75), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.625), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.60, WINSIZE.height * 0.625), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.50), false, 1, 1);
            break;
        case 11:
            // P:0 N:5 対角線と中央型
            putMedal(Vec2(WINSIZE.width * 0.20, WINSIZE.height * 0.90), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.20, WINSIZE.height * 0.79), false, 1, 3);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.50), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.80, WINSIZE.height * 0.21), false, 1, 3);
            putMedal(Vec2(WINSIZE.width * 0.80, WINSIZE.height * 0.10), false, 1, 1);
            break;
        case 12:
            // P:3 N:4 ひし形×２ 15
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.80), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.65), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.35), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.50), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.35), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.65), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.20), true , 1, 1);
            break;
        case 13:
            // P:1 N:3 なで肩のハンガー
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.85), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.75), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.38, WINSIZE.height * 0.55), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.62, WINSIZE.height * 0.55), false, 1, 1);
            break;
        case 14:
            // P:0 N:2 縦線
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.70), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.50), false, 1, 1);
            break;
        case 15:
            // P:1 N:3 四隅型 15
            putMedal(Vec2(WINSIZE.width * 0.15, WINSIZE.height * 0.80), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.10), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.90, WINSIZE.height * 0.90), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.85, WINSIZE.height * 0.20), false, 1, 1);
            break;
        case 16:
            // P:2 N:2 対角線 15
            putMedal(Vec2(WINSIZE.width * 0.20, WINSIZE.height * 0.80), false, 2, 2);
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.60), false, 2, 2);
            putMedal(Vec2(WINSIZE.width * 0.60, WINSIZE.height * 0.40), true , 2, 2);
            putMedal(Vec2(WINSIZE.width * 0.80, WINSIZE.height * 0.20), true , 2, 2);
            break;
        case 17:
            // P:2 N:1 端っこで斜め（高電荷・高質量） 15
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.795), true , 5, 12);
            putMedal(Vec2(WINSIZE.width * 0.275, WINSIZE.height * 0.90), true , 5, 12);
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.90), false, 5, 12);
            break;
        case 18:
            // P:3 N:2 五角形 21
            putMedal(Vec2(WINSIZE.width * 0.35, WINSIZE.height * 0.75), true , 3, 2);
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.85), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.65), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.60, WINSIZE.height * 0.85), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.65, WINSIZE.height * 0.75), true , 3, 2);
            break;
        case 19:
            // P:1 N:3 Y型
            putMedal(Vec2(WINSIZE.width * 0.75, WINSIZE.height * 0.70), false, 5, 1);
            putMedal(Vec2(WINSIZE.width * 0.25, WINSIZE.height * 0.70), false, 1, 3);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.50), true,  2, 2);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.30), false, 2, 7);
            break;
        case 20:
            // P:0 N:3 縦線（高質量隊） 20
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.80), false, 1, 6);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.60), false, 3, 8);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.40), false, 1, 6);
            break;
        case 21:
            // P:4 N:5 正方形（プラス電荷のバリア） 21
            putMedal(Vec2(WINSIZE.width * 0.33, WINSIZE.height * 0.705), false);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.705), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.67, WINSIZE.height * 0.705), false);
            putMedal(Vec2(WINSIZE.width * 0.33, WINSIZE.height * 0.600), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.600), false);
            putMedal(Vec2(WINSIZE.width * 0.67, WINSIZE.height * 0.600), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.33, WINSIZE.height * 0.495), false);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.495), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.67, WINSIZE.height * 0.495), false);
            break;
        case 22:
            // P:1 N:3 倒立のY型 22
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.75), true , 3, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.62), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.50), false, 3, 1);
            putMedal(Vec2(WINSIZE.width * 0.60, WINSIZE.height * 0.50), false, 3, 1);
            break;
        case 23:
            // P:0 N:2 横線 23
            putMedal(Vec2(WINSIZE.width * 0.75, WINSIZE.height * 0.6), false, 2, 2);
            putMedal(Vec2(WINSIZE.width * 0.25, WINSIZE.height * 0.6), false, 2, 2);
            break;
        case 24:
            // P:0 N:4 Y型 25
            putMedal(Vec2(WINSIZE.width * 0.35, WINSIZE.height * 0.85), false, 3, 1);
            putMedal(Vec2(WINSIZE.width * 0.65, WINSIZE.height * 0.85), false, 3, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.55), false, 1, 3);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.25), false, 1, 3);
            break;
        case 25:
            // P:1 N:3 隅で正方形（プラスの高電荷バリアー） 25
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.90), false, 1, 12);
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.795), false, 1, 12);
            putMedal(Vec2(WINSIZE.width * 0.275, WINSIZE.height * 0.90), false, 1, 12);
            putMedal(Vec2(WINSIZE.width * 0.275, WINSIZE.height * 0.795), true , 5, 12);
            break;
        case 26:
            // P:1 N:1 縦線（高電荷・高質量） 26
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.80), true,  2, 2);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.20), false, 2, 2);
            break;
        case 27:
            // P:3 N:3 正三角形 26
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.70), true);
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.60), false);
            putMedal(Vec2(WINSIZE.width * 0.60, WINSIZE.height * 0.60), false);
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.50), true);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.50), false);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.50), true);
            break;
        case 28:
            // P:2 N:2 正方形
            putMedal(Vec2(WINSIZE.width * 0.35, WINSIZE.height * 0.70), true);
            putMedal(Vec2(WINSIZE.width * 0.35, WINSIZE.height * 0.55), false);
            putMedal(Vec2(WINSIZE.width * 0.65, WINSIZE.height * 0.55), true);
            putMedal(Vec2(WINSIZE.width * 0.65, WINSIZE.height * 0.70), false);
            break;
        case 29:
            // P:2 N:3 四隅型 15
            putMedal(Vec2(WINSIZE.width * 0.15, WINSIZE.height * 0.80), false, 1, 3);
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.10), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.50), true , 3, 1);
            putMedal(Vec2(WINSIZE.width * 0.90, WINSIZE.height * 0.90), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.85, WINSIZE.height * 0.20), false, 1, 3);
            break;
        case 30:
            // P:3 N:1 ひし形（プラス粒子でバリアー） 32
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.75), true , 2, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.85), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.65), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.75), true , 2, 1);
            break;
        case 31:
            // P:2 N:2 布団たたき型
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.79), true , 3, 1);
            putMedal(Vec2(WINSIZE.width * 0.28, WINSIZE.height * 0.90), true , 3, 1);
            putMedal(Vec2(WINSIZE.width * 0.28, WINSIZE.height * 0.79), false, 1, 5);
            putMedal(Vec2(WINSIZE.width * 0.80, WINSIZE.height * 0.16), false, 1, 1);
            break;
        case 32:
            // P:0 N:3 ナナメ（高電荷・低質量と低電荷・超高質量のプラス粒子） 39
            putMedal(Vec2(WINSIZE.width * 0.13, WINSIZE.height * 0.87), false, 5, 1);
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.70), false, 2, 1);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.50), false, 1, 8);
            break;
        case 33:
            // P:1 N:4 横線型
            putMedal(Vec2(WINSIZE.width * 0.20, WINSIZE.height * 0.70), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.35, WINSIZE.height * 0.70), false, 1, 4);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.70), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.65, WINSIZE.height * 0.70), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.85, WINSIZE.height * 0.70), false, 1, 4);
            break;
        case 34:
            // P:4 N:4 正方形
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.40), true);
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.60), false);
            putMedal(Vec2(WINSIZE.width * 0.60, WINSIZE.height * 0.60), true);
            putMedal(Vec2(WINSIZE.width * 0.60, WINSIZE.height * 0.40), false);
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.90), true);
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.10), false);
            putMedal(Vec2(WINSIZE.width * 0.90, WINSIZE.height * 0.10), true);
            putMedal(Vec2(WINSIZE.width * 0.90, WINSIZE.height * 0.90), false);
            break;
        case 35:
            // P:2 N:2 台形型
            putMedal(Vec2(WINSIZE.width * 0.35, WINSIZE.height * 0.85), false, 1, 4);
            putMedal(Vec2(WINSIZE.width * 0.65, WINSIZE.height * 0.85), false, 1, 4);
            putMedal(Vec2(WINSIZE.width * 0.20, WINSIZE.height * 0.35), true , 3, 1);
            putMedal(Vec2(WINSIZE.width * 0.80, WINSIZE.height * 0.35), true , 3, 1);
            break;
        case 36:
            // P:4 N:4 対角線
            putMedal(Vec2(WINSIZE.width * 0.15, WINSIZE.height * 0.85), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.25, WINSIZE.height * 0.75), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.35, WINSIZE.height * 0.65), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.45, WINSIZE.height * 0.55), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.55, WINSIZE.height * 0.45), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.65, WINSIZE.height * 0.35), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.75, WINSIZE.height * 0.25), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.85, WINSIZE.height * 0.15), false, 1, 1);
            break;
        case 37:
            // P:2 N:2 正方形
            putMedal(Vec2(WINSIZE.width * 0.15, WINSIZE.height * 0.85), true , 1, 5);
            putMedal(Vec2(WINSIZE.width * 0.85, WINSIZE.height * 0.85), true , 3, 3);
            putMedal(Vec2(WINSIZE.width * 0.85, WINSIZE.height * 0.50), false, 2, 4);
            putMedal(Vec2(WINSIZE.width * 0.15, WINSIZE.height * 0.50), false, 4, 2);
            break;
        case 38:
            // P:2 N:4 エヌ型
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.75), false, 1, 4);
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.45), false, 1, 4);
            putMedal(Vec2(WINSIZE.width * 0.45, WINSIZE.height * 0.65), true , 3, 2);
            putMedal(Vec2(WINSIZE.width * 0.55, WINSIZE.height * 0.55), true , 3, 2);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.45), false, 1, 4);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.75), false, 1, 4);
            break;
        case 39:
            // P:1 N:2 ナナメ（高電荷・低質量と低電荷・超高質量のプラス粒子） 39
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.90), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.70), true , 2, 1);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.50), true , 1, 8);
            break;
        case 40:
            // P:2 N:2 横長ひし形（プラスがちょい重） 42
            putMedal(Vec2(WINSIZE.width * 0.25, WINSIZE.height * 0.61), true , 1, 2);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.67), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.55), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.75, WINSIZE.height * 0.61), true , 1, 2);
            break;
        case 41:
            // P:1 N:4 エックス型 34
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.75), false, 1, 4);
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.45), false, 1, 4);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.60), true , 1, 4);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.45), false, 1, 4);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.75), false, 1, 4);
            break;
        case 42:
            // P:0 N:2 対角線
            putMedal(Vec2(WINSIZE.width * 0.125, WINSIZE.height * 0.885), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.875, WINSIZE.height * 0.115), false, 1, 1);
            break;
        case 43:
            // P:2 N:3 槍型（高電荷・高質量）
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.795), true, 5, 12);
            putMedal(Vec2(WINSIZE.width * 0.275, WINSIZE.height * 0.90), true, 5, 12);
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.90), false, 5, 6);
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.60), false, 1, 12);
            putMedal(Vec2(WINSIZE.width * 0.80, WINSIZE.height * 0.20), false, 3, 1);
            break;
        case 44:
            // P:6 N:2 ダブリュー型
            putMedal(Vec2(WINSIZE.width * 0.20, WINSIZE.height * 0.80), true , 2, 2);
            putMedal(Vec2(WINSIZE.width * 0.25, WINSIZE.height * 0.70), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.38, WINSIZE.height * 0.43), false, 1, 18);
            putMedal(Vec2(WINSIZE.width * 0.40, WINSIZE.height * 0.55), false, 2, 6);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.65), true , 2, 2);
            putMedal(Vec2(WINSIZE.width * 0.60, WINSIZE.height * 0.55), false, 2, 6);
            putMedal(Vec2(WINSIZE.width * 0.62, WINSIZE.height * 0.43), false, 1, 24);
            putMedal(Vec2(WINSIZE.width * 0.75, WINSIZE.height * 0.70), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.80, WINSIZE.height * 0.80), true , 2, 2);
            break;
        case 45:
            // P:1 N:1 縦線（高電荷・高質量） 41
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.80), true,  5, 5);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.20), false, 5, 5);
            break;
        case 46:
            // P:6 N:2 エヌ型
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.80), true , 2, 1);
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.60), true , 1, 2);
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.40), true , 2, 1);
            putMedal(Vec2(WINSIZE.width * 0.42, WINSIZE.height * 0.70), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.58, WINSIZE.height * 0.50), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.40), true , 2, 1);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.60), true , 1, 2);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.80), true , 2, 1);
            break;
        case 47:
            // P:4 N:2 角に三角形ふたつ
            putMedal(Vec2(WINSIZE.width * 0.13, WINSIZE.height * 0.815), true, 5, 12);
            putMedal(Vec2(WINSIZE.width * 0.255, WINSIZE.height * 0.885), true, 5, 12);
            putMedal(Vec2(WINSIZE.width * 0.10, WINSIZE.height * 0.90), false, 2, 4);
            putMedal(Vec2(WINSIZE.width * 0.87, WINSIZE.height * 0.815),false, 5, 12);
            putMedal(Vec2(WINSIZE.width * 0.745, WINSIZE.height * 0.885),false, 5, 12);
            putMedal(Vec2(WINSIZE.width * 0.90, WINSIZE.height * 0.90), true , 4, 4);
            break;
        case 48:
            // P:3 N:1 ひし形（プラス粒子でバリアー） 48
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.75), true , 4, 2);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.85), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.65), true , 1, 5);
            putMedal(Vec2(WINSIZE.width * 0.70, WINSIZE.height * 0.75), true , 4, 2);
            break;
        case 49:
            // P:2 N:2 縦線 49
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.75), true , 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.60), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.45), false, 1, 1);
            putMedal(Vec2(WINSIZE.width * 0.50, WINSIZE.height * 0.30), true , 1, 1);
            break;
        case 50:
            // P:0 N:2 端っこで斜め（高電荷・高質量） 50
            putMedal(Vec2(WINSIZE.width * 0.125, WINSIZE.height * 0.79), false, 3, 1);
            putMedal(Vec2(WINSIZE.width * 0.30, WINSIZE.height * 0.885), false, 3, 1);
            break;
        default:
            break;
    }
}

void QuestScene::fieldColor(){
    // 場の電位を計算し、色で表現
    float WINLENGTH = sqrt(WINSIZE.width * WINSIZE.width + WINSIZE.height * WINSIZE.height);
    for(auto bg : _vBackground){
        auto color = bg->getColor();

        if(color.r < 8 && color.b < 8){
            continue;
        }

        int red   = 0;
        int green = 0;
        int blue  = 0;

        for(auto medal : _vMedal){
            if(medal->isDead()){
                continue;
            }

            Vec2 r = medal->getPosition() - bg->getPosition();
            float len = sqrt(r.x * r.x + r.y * r.y);
            float charge = medal->getCharge() * WINLENGTH / len;

            if(medal->getType() > 0){
                red += charge;
            }
            else if(medal->getType() < 0){
                blue += charge;
            }
        }

        if(red > blue){
            red -= blue;
            if(red > 31)
                red = 31;
            bg->setColor(Color3B(8 * red, 0, 0));
            bg->setSkewX(red);
            bg->setSkewY(red);
        }
        else if(blue > red){
            blue -= red;
            if(blue > 31)
                blue = 31;
            bg->setColor(Color3B(0, 0, 8 * blue));

            bg->setSkewX(blue);
            bg->setSkewY(blue);
        }
    }
}

void QuestScene::initMenu(){
    auto label1 = Label::createWithSystemFont("戻る", "fonts/Marker Felt.ttf", 64);
    label1->enableShadow(Color4B::GRAY, Size(0.5, 0.5), 15);
    label1->enableOutline(Color4B::BLUE, 5);
    auto item1 = MenuItemLabel::create(label1, [&](Ref* pSender) {
        //指定秒数後に次のシーンへ
        if(_state == GameStatus::Playing)
            backTitle(1.0f);
    });

    auto label2 = Label::createWithSystemFont("ヒント", "fonts/Marker Felt.ttf", 64);
    label2->enableShadow(Color4B::GRAY, Size(0.5, 0.5), 15);
    label2->enableOutline(Color4B::BLUE, 5);
    auto item2 = MenuItemLabel::create(label2, [&](Ref* pSender) {
        //指定秒数後に次のシーンへ
        if(_state == GameStatus::Playing)
            showHint("ヒント");
    });

    auto menu = Menu::create(item1, item2, NULL);
    menu->setPosition(Vec2(WINSIZE.width * 0.5, WINSIZE.height * 0.03));
    menu->alignItemsHorizontallyWithPadding(WINSIZE.width * 0.5);
    this->addChild(menu, ZOrder::Background);
}

void QuestScene::initLevelLayer() {
    CCLOG("initLevelLayer");
    CCLOG("WINSIZE.width:  %f", WINSIZE.width);
    CCLOG("WINSIZE.height: %f", WINSIZE.height);

    //レベルレイヤーの生成
    auto levelLayer = LayerColor::create(Color4B(0, 0, 0, 0), WINSIZE.width, WINSIZE.height);
    levelLayer->setPosition(Point::ZERO);
    levelLayer->setTag(TAG_LEVEL_LAYER);
    addChild(levelLayer, ZOrder::Level);

    //レベルの表示
    auto levelSprite = Sprite::create("Level.png");
    levelSprite->setPosition(Point(WINSIZE.width * 0.4, WINSIZE.height * 0.5));
    levelLayer->addChild(levelSprite);

    //レベル数の表示
    //1の位の表示
    auto levelNumPath_1 = StringUtils::format("%d.png", _level % 10);
    auto levelNumSprite_1 = Sprite::create(levelNumPath_1.c_str());
    levelNumSprite_1->setPosition(Point(WINSIZE.width * 0.7, WINSIZE.height * 0.5));
    levelLayer->addChild(levelNumSprite_1);
    //10の位の表示
    if(_level >= 10){
        int level_10 = _level / 10;
        auto levelNumPath_10 = StringUtils::format("%d.png", level_10);
        auto levelNumSprite_10 = Sprite::create(levelNumPath_10.c_str());
        levelNumSprite_10->setPosition(Point(WINSIZE.width * 0.7 - 1.35 * levelNumSprite_1->getContentSize().width, WINSIZE.height * 0.5));
        levelLayer->addChild(levelNumSprite_10);
    }

    // 1秒後に消えるようにする
    scheduleOnce(schedule_selector(QuestScene::removeLevelLayer), 1.0f);
}

//レベル表示レイヤーの削除
void QuestScene::removeLevelLayer(float dt) {
    CCLOG("removeLevelLayer");

    //0.5秒で消えるようにする
    auto levelLayer = getChildByTag(TAG_LEVEL_LAYER);
    levelLayer->runAction(Sequence::create(
                                FadeTo::create(0.5, 0),
                                RemoveSelf::create(),
                                nullptr));

    _state = GameStatus::Playing;
    _touchable = true;
}

void QuestScene::initTutorial(){
    if(_level > 5){
        return;
    }

    showHint("チュートリアル", true);
}

void QuestScene::showHint(std::string title){
    showHint(title, false);
}

void QuestScene::showHint(std::string title, bool actionFlg){
    auto text = getText();

    cocos2d::ccMenuCallback action = CC_CALLBACK_1(QuestScene::menuStartCallback,this);
    std::vector<UIDialogButton*> buttons = {
        new UIDialogButton("OK",action,1),
    };
    auto* dialog = UIDialog::create(title, text.c_str(), buttons);

    if(actionFlg){
        dialog->setScale(0.0f);
        dialog->runAction(Sequence::create(
                Hide::create(),
                DelayTime::create(1.0f),
                Show::create(),
                ScaleTo::create(0.5f, 1.0f),
                nullptr));
    }

    addChild(dialog, 100, T_DIALOG);
}

std::string QuestScene::getText(){
    std::string text;

    switch(_level){
        case  1:
            text += "画面をタップしてプラスの粒子を\n";
            text += "配置しよう。\n";
            text += "\n";
            text += "プラスとマイナスは引き合うぞ！\n";
            text += "すべて消すとステージクリアだ！\n";
            break;
        case  2:
            text += "タップし続けると矢印が出て\n";
            text += "好きな方向に粒子を飛ばせるぞ！\n";
            text += "\n";
            text += "狙いを定めてマイナスを消そう。";
            break;
        case  3:
            text += "同じ色同士は反発するぞ！\n";
            text += "場外に弾き飛ばすと消えるぞ！";
            break;
        case  4:
            text += "自分の粒子は場外にならない！\n";
            text += "マイナスとぶつけて消そう。";
            break;
        case  5:
            text += "粒子に書かれた数字は\n";
            text += "上が電荷、下が質量だ！\n";
            text += "\n";
            text += "電荷の数字が大きいほど、\n";
            text += "大きな引力・斥力が作用するぞ！\n";
            text += "\n";
            text += "質量の数字が大きいほど、\n";
            text += "重くて動きにくいんだ！\n";
            break;
        case  6:
            text += "マイナスの動く方向を予測して\n";
            text += "プラスを配置してみよう！\n";
            break;
        case  7:
            text += "自分のプラス粒子で\n";
            text += "マイナスを消してみよう！\n";
            break;
        case  8:
            text += "自分のプラス粒子で\n";
            text += "マイナスを消してみよう。\n";
            break;
        case  9:
            text += "画面下部のマイナスは\n";
            text += "重くて動きにくいぞ。\n";
            text += "\n";
            text += "場外しにくいマイナスは\n";
            text += "自分のプラスで消してしまおう！\n";
            break;
        case 10:
            text += "画面下部に\n";
            text += "プラス粒子を配置してみよう！\n";
            break;
        case 11:
            text += "画面中央のマイナスを消してみよう。\n";
            break;
        case 12:
            text += "プラスが3個、マイナスが4個ある。\n";
            text += "\n";
            text += "自分のプラス粒子を1個置いたら、\n";
            text += "ピッタリ全部消せるはず！\n";
            break;
        case 13:
            text += "右下隅にプラス粒子を\n";
            text += "配置してみよう！\n";
            break;
        case 14:
            text += "二つのマイナスの間を\n";
            text += "勢いよく横切るように、\n";
            text += "自分のプラスをハジき飛ばそう！\n";
            break;
        case 15:
            text += "画面中央に\n";
            text += "プラス粒子を配置してみよう！\n";
            break;
        case 16:
            text += "対角線上に\n";
            text += "プラス粒子を配置してみよう！\n";
            text += "\n";
            text += "＜ワンポイントアドバイス＞\n";
            text += "マイナス粒子の近くで\n";
            text += "タップし、弾き飛ばす前に\n";
            text += "マイナス粒子に重ねると・・・。\n";
            break;
        case 17:
            text += "矢印の方向をチェック！\n";
            text += "\n";
            text += "左上隅のマイナスは\n";
            text += "画面中央に向かって\n";
            text += "勢いよく飛び出してくるぞ！\n";
            break;
        case 18:
            text += "プラスの電場が強いため、\n";
            text += "五角形の外側から狙っても\n";
            text += "跳ね返されてしまうぞ！\n";
            text += "\n";
            text += "自分のプラス粒子で\n";
            text += "マイナス粒子を消せるような\n";
            text += "場所を探そう！\n";
            break;
        case 19:
            text += "左上のマイナスを場外させると\n";
            text += "クリアしやすいぞ！\n";
            text += "\n";
            text += "画面中段、左の壁際に\n";
            text += "プラス粒子を配置してみよう。\n";
            break;
        case 20:
            text += "マイナス同士が反発しあう性質を\n";
            text += "利用しよう！\n";
            text += "\n";
            text += "できるだけ時間をかせいで\n";
            text += "マイナスを場外させるんだ！\n";
            break;
        case 21:
            text += "プラスが4個、マイナスが5個ある。\n";
            text += "\n";
            text += "自分のプラス粒子を1個置いたら、\n";
            text += "ピッタリ全部消せるはず！\n";
            break;
        case 22:
            text += "ここは動きが速いステージだ！\n";
            text += "\n";
            text += "流れに逆らわず\n";
            text += "自分のプラス粒子を消せる場所は？\n";
            break;
        case 23:
            text += "シンプルなステージだが\n";
            text += "意外と難しいかも。\n";
            text += "\n";
            text += "マイナス粒子の片方だけ場外させ、\n";
            text += "残りを自分のプラスで消すには\n";
            text += "どうすればいい？\n";
            break;
        case 24:
            text += "一番下のマイナスと下の壁の間を\n";
            text += "すり抜けるように、\n";
            text += "自分のプラスをハジいてみよう！\n";
            break;
        case 25:
            text += "画面右下隅に\n";
            text += "プラス粒子を配置してみよう！\n";
            break;
        case 26:
            text += "画面上部のプラスを場外させよう！\n";
            text += "\n";
            text += "下から全力でぶつければ\n";
            text += "きっと何とかできるはず！\n";
            break;
        case 27:
            text += "画面右下隅に\n";
            text += "プラス粒子を配置してみよう！\n";
            text += "\n";
            text += "＜ワンポイントアドバイス＞\n";
            text += "プラス粒子の近くでタップし、\n";
            text += "弾き飛ばす前にピッタリ重ねると、\n";
            text += "モノ凄い反発力が発生するぞ！\n";
            text += "\n";
            text += "自分のプラスは場外しないので、\n";
            text += "使いどころに気を付けよう。\n";
            break;
        case 28:
            text += "プラス粒子を1つ場外させよう！\n";
            text += "\n";
            text += "＜ワンポイントアドバイス＞\n";
            text += "プラス粒子の近くでタップし、\n";
            text += "弾き飛ばす前にピッタリ重ねると、\n";
            text += "モノ凄い反発力が発生するぞ！\n";
            text += "\n";
            text += "自分のプラスは場外しないので、\n";
            text += "使いどころに気を付けよう。\n";
            break;
        case 29:
            text += "このステージはとても難しい。\n";
            text += "もしもクリアできたら超スゴいぞ！\n";
            text += "\n";
            text += "（念のため言っておくと、\n";
            text += "　もちろんクリアはできるぞ！）\n";
            break;
        case 30:
            text += "自分のプラス粒子で\n";
            text += "マイナスを消してしまおう！\n";
            break;
        case 31:
            text += "右下のマイナスを場外させよう。\n";
            text += "\n";
            text += "右下のマイナスと壁の間を\n";
            text += "通るようにプラス粒子をハジこう！\n";
            text += "\n";
            text += "もし難しければ、\n";
            text += "左下にプラスを配置しよう！\n";
            break;
        case 32:
            text += "矢印の大きさをチェック！\n";
            text += "\n";
            text += "場外しにくい粒子は\n";
            text += "自分のプラスで消してしまおう！\n";
            break;
        case 33:
            text += "ここはクセがあるステージだ。\n";
            text += "\n";
            text += "どのマイナスを消しても\n";
            text += "1つ残ってしまうようになってるぞ。\n";
            text += "\n";
            text += "どうすればクリアできるかな？\n";
            break;
        case 34:
            text += "プラス粒子を1つ場外させよう！\n";
            text += "\n";
            text += "＜ワンポイントアドバイス＞\n";
            text += "プラス粒子の近くでタップし、\n";
            text += "弾き飛ばす前にピッタリ重ねると、\n";
            text += "モノ凄い反発力が発生するぞ！\n";
            text += "\n";
            text += "自分のプラスは場外しないので、\n";
            text += "使いどころに気を付けよう。\n";
            break;
        case 35:
            text += "画面右下隅に\n";
            text += "プラス粒子を配置してみよう！\n";
            break;
        case 36:
            text += "このステージは、意外と簡単だ！\n";
            break;
        case 37:
            text += "左上のプラス粒子を場外させると\n";
            text += "クリアしやすいぞ！\n";
            text += "\n";
            text += "プラス同士が反発しあう性質を\n";
            text += "利用するんだ！\n";
            break;
        case 38:
            text += "このステージはとても難しい。\n";
            text += "もしもクリアできたら超スゴいぞ！\n";
            text += "\n";
            text += "（念のため言っておくと、\n";
            text += "　もちろんクリアはできるぞ！）\n";
            break;
        case 39:
            text += "左上のマイナスは自分のプラスで\n";
            text += "消す必要があるぞ！\n";
            text += "\n";
            text += "残りの2つのプラスは、\n";
            text += "同時に場外させるため、\n";
            text += "十分に近づくようにしよう。\n";
            text += "\n";
            text += "答えは対角線上にあるはず！\n";
            break;
        case 40:
            text += "プラス粒子を1つ場外させよう！\n";
            text += "\n";
            text += "＜ワンポイントアドバイス＞\n";
            text += "プラス粒子の近くでタップし、\n";
            text += "弾き飛ばす前にピッタリ重ねると、\n";
            text += "モノ凄い反発力が発生するぞ！\n";
            text += "\n";
            text += "自分のプラスは場外しないので、\n";
            text += "使いどころに気を付けよう。\n";
            break;
        case 41:
            text += "中央のプラスと右上のマイナスの\n";
            text += "ちょうど真ん中に\n";
            text += "自分のプラスを配置してみよう！\n";
            break;
        case 42:
            text += "シンプルなステージだが\n";
            text += "意外と難しいかも。\n";
            text += "\n";
            text += "マイナス粒子の片方だけ場外させ、\n";
            text += "残りを自分のプラスで消すには\n";
            text += "どうすればいい？\n";
            break;
        case 43:
            text += "同じ色同士が反発しあう性質を\n";
            text += "利用しよう！\n";
            text += "\n";
            text += "できるだけ時間をかせいで\n";
            text += "中央のマイナスを最後に消そう！\n";
            break;
        case 44:
            text += "このステージは、\n";
            text += "どのマイナスを消しても\n";
            text += "必ず1つ残るように作ったぞ！\n";
            text += "\n";
            text += "・・・と思ったんだけど、\n";
            text += "実はあそこのマイナスを消すと\n";
            text += "簡単にクリアできちゃうんです。\n";
            break;
        case 45:
            text += "プラス粒子を場外させよう！\n";
            text += "\n";
            text += "＜ワンポイントアドバイス＞\n";
            text += "プラス粒子の近くでタップし、\n";
            text += "弾き飛ばす前にピッタリ重ねると、\n";
            text += "モノ凄い反発力が発生するぞ！\n";
            text += "\n";
            text += "自分のプラスは場外しないので、\n";
            text += "使いどころに気を付けよう。\n";
            break;
        case 46:
            text += "このステージは結構ムズい！\n";
            text += "\n";
            text += "真ん中のプラスを場外させると\n";
            text += "クリアしやすいと思うぞ！\n";
            break;
        case 47:
            text += "隅にあるプラスとマイナスは\n";
            text += "それぞれ画面中央に向かって\n";
            text += "勢いよく飛び出してくるぞ！\n";
            text += "\n";
            text += "マイナスが飛び出してくる場所に\n";
            text += "ぴったり自分のプラスを\n";
            text += "配置してみよう！\n";
            break;
        case 48:
            text += "重いプラス粒子を場外させよう！\n";
            text += "\n";
            text += "＜ワンポイントアドバイス＞\n";
            text += "プラス粒子の近くでタップし、\n";
            text += "弾き飛ばす前にピッタリ重ねると、\n";
            text += "モノ凄い反発力が発生するぞ！\n";
            text += "\n";
            text += "自分のプラスは場外しないので、\n";
            text += "使いどころに気を付けよう。\n";
            break;
        case 49:
            text += "プラス粒子を1つ場外させよう！\n";
            text += "\n";
            text += "＜ワンポイントアドバイス＞\n";
            text += "プラス粒子の近くでタップし、\n";
            text += "弾き飛ばす前にピッタリ重ねると、\n";
            text += "モノ凄い反発力が発生するぞ！\n";
            text += "\n";
            text += "自分のプラスは場外しないので、\n";
            text += "使いどころに気を付けよう。\n";
            break;
        case 50:
            text += "ここが最終ステージだ！\n";
            text += "\n";
            text += "ここまでたどり着いたキミは\n";
            text += "本当に素晴らしい！\n";
            text += "\n";
            text += "最後のヒントは、\n";
            text += "あえて言わないことにする。\n";
            text += "\n";
            text += "健闘を祈る！\n";
            break;
        default:
            break;
    }

    return text;
}
void QuestScene::menuStartCallback(Ref* Sender)
{
    MenuItem* menuItem = (MenuItem*)Sender;
    log("%d",menuItem->getTag());

    switch(menuItem->getTag()){
        case 1:
            break;
    }

    dialogClose();
}

void QuestScene::dialogClose()
{
    UIDialog* dialog = static_cast<UIDialog*>(getChildByTag(T_DIALOG));
    dialog->close();
}


void QuestScene::update(float dt) {
    if (_state == GameStatus::Playing) {
        // touchBegin時に_touchableをfalseに変更される。
        // プレイヤーがメダルを置くたびにメダルを移動させる。
        if(_touchable == false){
            CCLOG("calculating...");

            // メダル同士が重なっていた場合、Typeが異なる（＋と－）の場合、メダルを消去する。
            for(auto medal : _vMedal){
                if(medal->isDead()){
                    continue;
                }

                for(auto medal2 : _vMedal){
                    if(medal->isTouchPoint(medal2->getPosition())
                        && medal->getType() != medal2->getType()){
                        destroyEffect(medal2);
                        destroyEffect(medal);
                    }
                }
            }

            // 各メダルの速度ベクトルを計算
            // 速度ベクトル = Σf(対象メダルの座標, 対象メダルの符号, 他メダルの座標, 他メダルの符号)
            int totalVelocity = 0;
            for(auto medal : _vMedal){
                if(medal->isDead()){
                    continue;
                }

                medal->calculateVelocity(_vMedal);
                totalVelocity += medal->getAbsVelocity();
            }

            // 各メダルに引き合う力、反発しあう力のエフェクトを実行
            for(auto medal : _vMedal){

            }

            // 各メダルを速度ベクトルにしたがって移動させる。
            for(auto medal : _vMedal){
                if(medal->isDead()){
                    continue;
                }

                if(medal->getAbsVelocity() > 0){
                    medal->runAction(CCMoveBy::create(dt, medal->getVelocity() / 10000));
                }
                else{
                    medal->stopAllActions();
                }
            }

            // メダルの消去判定
            // 各メダルがフィールドから外に出たら消去エフェクトを実行
            for(auto medal : _vMedal){
                if(medal->isDead()){
                    continue;
                }

                if(isOutOfBattleField(medal)){
                    if(medal->isMyMedal()){
                        //手玉の場合は壁で反射する
                        reflectVelocity(medal);
                    }
                    else{
                        //手玉以外の場合は場外で消去
                        destroyEffect(medal);
                    }
                }
            }

            // 場の背景色を変える。O(n2)
            fieldColor();

            // ターン終了判定
            // 全メダルの移動ベクトルが0になった場合、ターン終了する。
            // ターン終了時に_touchableをtrueに変更する。
            if(totalVelocity == 0){
                CCLOG("_touchable: ON!");
                _touchable = true;
                _pFlg = !_pFlg;
                _putMedalNum++;
            }
            CCLOG("totalVelocity: %i", totalVelocity);

            // ステージクリア判定
            if(isStageClearInLoop()){
                _state = GameStatus::Result;
            }
            //ステージ失敗判定
            else if(isStageFailedInLoop()){
                _state = GameStatus::Failed;
            }
        }
        //ステージクリア判定
        // ステージクリアしていたらGameStatusをResultに更新
        if(isStageClear()){
            _state = GameStatus::Result;
        }
        // ステージ失敗判定
        else if(isStageFailed()){
            _state = GameStatus::Failed;
        }
    }
    else if (_state == GameStatus::Result) {
        _touchable = false;
        _state = GameStatus::Ready;

        //保存先の生成
        UserDefault *userDef = UserDefault::getInstance();

        // クリアしたステージ情報を登録
        std::string keyIsClear = StringUtils::format("IS_CLEAR_STAGE%2d", _level);
        userDef->setBoolForKey(keyIsClear.c_str(), true);
        userDef->flush();

        // 解放されるステージがあるかチェック（直近10ステージのうち、7つ以上クリアしているか？）
        int counter = 0;
        int checkRangeFrom = (_level - 1) / 10;
        checkRangeFrom *= 10;
        checkRangeFrom += 1;

        for(int i = 0; i < 10; i++){
            int checkLevel = checkRangeFrom + i;
            std::string chkKeyIsClear = StringUtils::format("IS_CLEAR_STAGE%2d", checkLevel);
            bool isClear = userDef->getBoolForKey(chkKeyIsClear.c_str(), false);
            if(isClear){
                counter++;
            }
        }

        // 解放されるステージがある場合は解放するステージ情報を登録
        if(counter >= 7 && (checkRangeFrom + 20 - 1) <= MAX_LEVEL){
            int openRangeFrom = checkRangeFrom + 10;

            std::string key = StringUtils::format("IS_OPEN_STAGE%2d", openRangeFrom);
            if(!userDef->getBoolForKey(key.c_str(), false)){
                // 新しいステージが解放されたメッセージ
                std::string text;
                text += "直近10ステージのうち、\n";
                text += "7ステージクリアすると\n";
                text += "新ステージが10個開放されます。\n";
                cocos2d::ccMenuCallback action = CC_CALLBACK_1(QuestScene::menuStartCallback,this);
                std::vector<UIDialogButton*> buttons = {
                    new UIDialogButton("OK",action,1),
                };
                auto* dialog = UIDialog::create("新ステージが開放されました！", text.c_str(), buttons);

                dialog->setScale(0.0f);
                dialog->runAction(Sequence::create(
                        Hide::create(),
                        DelayTime::create(1.0f),
                        Show::create(),
                        ScaleTo::create(0.5f, 1.0f),
                        nullptr));

                addChild(dialog, ZOrder::Dialog, T_DIALOG);
            }

            for(int i = 0; i < 10; i++){
                int openLevel = openRangeFrom + i;
                std::string keyIsOpen = StringUtils::format("IS_OPEN_STAGE%2d", openLevel);
                userDef->setBoolForKey(keyIsOpen.c_str(), true);

                // 更新したステージ情報を保存
                userDef->flush();
            }
        }

        // updateメソッドを停止する
        this->unscheduleUpdate();

        CallFunc* func;
        func = CallFunc::create(CC_CALLBACK_0(QuestScene::resultAnimation, this));
        runAction(Sequence::create( func, nullptr));
    }
    else if(_state == GameStatus::Ready){
        // 何もしない
    }
    else if(_state == GameStatus::Failed){
        _touchable = false;
        _state = GameStatus::Ready;

        // updateメソッドを停止する
        this->unscheduleUpdate();

        CallFunc* func;
        func = CallFunc::create(CC_CALLBACK_0(QuestScene::failedAnimation, this));
        runAction(Sequence::create(
                func, nullptr));
    }
}

// 場外ならtrueを返す。
bool QuestScene::isOutOfBattleField(EnemySprite* medal){
    return isOutOfBattleField(medal->getPosition());
}

bool QuestScene::isOutOfBattleField(Vec2 pos){
    if(     pos.x > WINSIZE.width  * 0.9f
         || pos.x < WINSIZE.width  * 0.1f
         || pos.y > WINSIZE.height * 0.9f
         || pos.y < WINSIZE.height * 0.1f){
        return true;
    }
    return false;
}

void QuestScene::reflectVelocity(EnemySprite* medal){
    auto velocity = medal->getVelocity();

    // 左右の壁に当たった場合、横方向の速度を反転
    if(medal->getPositionX() < WINSIZE.width * 0.1){
        medal->setPosition(Vec2(WINSIZE.width * 0.11, medal->getPositionY()));
        velocity.x *= -1;
    }
    if(medal->getPositionX() > WINSIZE.width * 0.9){
        medal->setPosition(Vec2(WINSIZE.width * 0.89, medal->getPositionY()));
        velocity.x *= -1;
    }

    // 上下の壁に当たった場合、縦方向の速度を反転
    if(medal->getPositionY() < WINSIZE.height * 0.1){
        medal->setPosition(Vec2(medal->getPositionX(), WINSIZE.height * 0.11));
        velocity.y *= -1;
    }
    if(medal->getPositionY() > WINSIZE.height * 0.9){
        medal->setPosition(Vec2(medal->getPositionX(), WINSIZE.height * 0.89));
        velocity.y *= -1;
    }

    medal->setVelocity(velocity);
}

void QuestScene::destroyEffect(EnemySprite* medal){
    // メダルの破壊エフェクト
    auto p = _pool->pop();
    if(p != nullptr){
        CCPoint pos = ccp(medal->getPositionX(), medal->getPositionY());
        p->setPosition(pos);
        p->setAutoRemoveOnFinish(true); // 表示が終わったら自分を親から削除！
        this->addChild(p, Effect);
    }

    // メダルを画面外に移動
    medal->setPosition(-1000, -1000);
    medal->isDead(true);
}

bool QuestScene::isStageClear(){
    bool ret = true;
    switch(_level){
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        default:
            // すべてのメダルを消去する
            for(auto medal : _vMedal){
                if(!medal->isDead()){
                    ret = false;
                }
            }
            break;
    }
    return ret;
}

bool QuestScene::isStageClearInLoop(){
    bool ret = true;
    switch(_level){
        default:
            ret = false;
            break;
    }
    return ret;
}

bool QuestScene::isStageFailed(){
    bool ret = false;
    switch(_level){
        case  1:
        case  2:
        case  3:
        case  4:
        case  5:
        case  6:
        case  7:
        case  8:
        case  9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        default:
            if(_putMedalNum > 0){
                ret = true;
            }
            break;
    }
    return ret;
}

bool QuestScene::isStageFailedInLoop(){
    bool ret = false;
    switch(_level){
        default:
            break;
    }
    return ret;
}

//Resultアニメーション
void QuestScene::resultAnimation() {
    CCLOG("resultAnimation");

    //白い背景を用意する
    addColorLayer(Color4B(255, 255, 255, 127));

    // クリアのロゴを表示
    showSpriteAndSlideInTop("clear.png");

    // ゲーム終了時、一定の確率で広告表示
    if(CCRANDOM_0_1() < 0.05f){
        platform::NativeBridge::executeNative();
    }

    auto label1 = Label::createWithSystemFont("次へ", "fonts/Marker Felt.ttf", 64);
    label1->enableShadow(Color4B::GRAY, Size(0.5, 0.5), 15);
    label1->enableOutline(Color4B::BLUE, 5);
    auto item1 = MenuItemLabel::create(label1, [&](Ref* pSender) {
        _nextLevel = _level + 1;

        //指定秒数後に次のシーンへ
        nextScene(1.0f);
    });

    auto label2 = Label::createWithSystemFont("もう一回", "fonts/Marker Felt.ttf", 64);
    label2->enableShadow(Color4B::GRAY, Size(0.5, 0.5), 15);
    label2->enableOutline(Color4B::BLUE, 5);
    auto item2 = MenuItemLabel::create(label2, [&](Ref* pSender) {
        // 次のレベルを設定
        _nextLevel = _level;

        //指定秒数後に次のシーンへ
        nextScene(1.0f);
    });

    auto label3 = Label::createWithSystemFont("戻る", "fonts/Marker Felt.ttf", 64);
    label3->enableShadow(Color4B::GRAY, Size(0.5, 0.5), 15);
    label3->enableOutline(Color4B::BLUE, 5);
    auto item3 = MenuItemLabel::create(label3, [&](Ref* pSender) {
        //指定秒数後に次のシーンへ
        backTitle(1.0f);
    });

    auto menu = Menu::create();

    //次のステージがオープンしているかチェック
    UserDefault *userDef = UserDefault::getInstance();
    std::string keyIsOpen = StringUtils::format("IS_OPEN_STAGE%2d", _level + 1);
    bool nextStageIsOpen = userDef->getBoolForKey(keyIsOpen.c_str(), false);

    if(_level < MAX_LEVEL && nextStageIsOpen){
        menu = Menu::create(item1, item2, item3, NULL);
    }
    else{
        menu = Menu::create(item2, item3, NULL);
    }
    menu->alignItemsVerticallyWithPadding(50);
    this->addChild(menu, Level);
    menu->runAction(Sequence::create(
                                        CCHide::create(),
                                        DelayTime::create(1.5f),
                                        CCShow::create(),
                                        nullptr));
}

//Failedアニメーション
void QuestScene::failedAnimation() {
    CCLOG("failedAnimation");

    // 黒い背景を用意する
    addColorLayer(Color4B(0, 0, 0, 127));

    // 失敗のロゴを表示
    showSpriteAndSlideInTop("failed.png");

    // 次のレベルを設定
    _nextLevel = _level;

    auto label1 = Label::createWithSystemFont("リトライ", "fonts/Marker Felt.ttf", 64);
    label1->enableShadow(Color4B::GRAY, Size(0.5, 0.5), 15);
    label1->enableOutline(Color4B::BLUE, 5);
    auto item1 = MenuItemLabel::create(label1, [&](Ref* pSender) {
        //指定秒数後に次のシーンへ
        nextScene(1.0f);
    });

    auto label2 = Label::createWithSystemFont("戻る", "fonts/Marker Felt.ttf", 64);
    label2->enableShadow(Color4B::GRAY, Size(0.5, 0.5), 15);
    label2->enableOutline(Color4B::BLUE, 5);
    auto item2 = MenuItemLabel::create(label2, [&](Ref* pSender) {
        //指定秒数後に次のシーンへ
        backTitle(1.0f);
    });

    auto menu = Menu::create(item1, item2, NULL);
    menu->alignItemsVerticallyWithPadding(50);
    this->addChild(menu, Level);
    menu->runAction(Sequence::create(
                                        CCHide::create(),
                                        DelayTime::create(1.5f),
                                        CCShow::create(),
                                        nullptr));
}

void QuestScene::addColorLayer(Color4B color){
    // 背景を用意する
    auto layer = LayerColor::create(color, WINSIZE.width, WINSIZE.height);
    layer->setPosition(Point::ZERO);
    addChild(layer, Level);
}

void QuestScene::showSpriteAndSlideInTop(std::string filepath){
    auto sprite = CCSprite::create(filepath);
    sprite->setPosition(ccp(WINSIZE.width / 2, WINSIZE.height + sprite->getContentSize().height));
    addChild(sprite, ZOrder::Level);
    sprite->runAction(RepeatForever::create(
                        Sequence::create(
                                FadeTo::create(1.0f, 128),
                                FadeTo::create(1.0f, 255),
                                nullptr)));
    sprite->runAction(Sequence::create(
                                MoveTo::create(0.5f, ccp(WINSIZE.width * 0.5, WINSIZE.height * 0.7)),
                                nullptr));
}

//次のシーンへ遷移
void QuestScene::nextScene(float dt) {
    CCLOG("nextScene");

    if(!_changeScene){
        _changeScene = true;

        // 次のシーンを生成する
        auto scene = QuestScene::createScene(_nextLevel, 0.0f);
        Director::getInstance()->replaceScene(
                    CCTransitionSlideInL::create(dt, scene)
                );
    }
}

//次のシーンへ遷移
void QuestScene::backTitle(float dt) {
    CCLOG("backTitle");

    // 次のシーンを生成する
    auto scene = StageSelectScene::createScene();
    Director::getInstance()->replaceScene(
            TransitionFade::create(dt, scene, Color3B::BLACK)
    );
}
