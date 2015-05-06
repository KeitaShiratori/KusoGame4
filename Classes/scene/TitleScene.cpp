#include "TitleScene.h"
#include "StageSelectScene.h"

#define WINSIZE Director::getInstance()->getWinSize()
#define T_DIALOG 1000
#define STAGE_SELECT_SCENE 1

USING_NS_CC;

//コンストラクタ
TitleScene::TitleScene()
{

}

//シーン生成
Scene* TitleScene::createScene() {
    auto scene = Scene::create();
    auto layer = TitleScene::create();
    scene->addChild(layer);
    return scene;
}

//インスタンス生成
TitleScene* TitleScene::create() {
    TitleScene *pRet = new TitleScene();
    pRet->init();
    pRet->autorelease();

    return pRet;
}

//初期化
bool TitleScene::init() {
    if (!Layer::init())
        return false;

    // パスを追加
    CCFileUtils::sharedFileUtils()->addSearchPath("extensions");

    // 湯気エフェクトのプール
    _pool = ParticleSystemPool::create("yuge_texture.plist", 20);
    _pool->retain();

    // シングルタップイベントの取得
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(_swallowsTouches);
    touchListener->onTouchBegan = CC_CALLBACK_2(TitleScene::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(TitleScene::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(TitleScene::onTouchEnded, this);
    touchListener->onTouchCancelled = CC_CALLBACK_2(TitleScene::onTouchCancelled, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    initTitle(); //タイトルの初期化
    initMenu();

    // 初回起動時に限り、ステージオープン情報を初期化する
    UserDefault *userDef = UserDefault::getInstance();

    if(!userDef->getBoolForKey("IS_OPEN_STAGE1", false)){
        for(int i = 1; i <= 10; i++){
            std::string keyIsOpen = StringUtils::format("IS_OPEN_STAGE%2d", i);
            userDef->setBoolForKey(keyIsOpen.c_str(), true);

            // 更新したステージ情報を保存
            userDef->flush();
        }
    }

    return true;
}


bool TitleScene::onTouchBegan(Touch* touch, Event* unused_event) {
    CCLOG("onToucnBegan");

    auto p = _pool->pop();
    if(p != nullptr){
        CCPoint pos = ccp(touch->getLocation().x, touch->getLocation().y);
        p->setPosition(pos);
        p->setAutoRemoveOnFinish(true); // 表示が終わったら自分を親から削除！
        this->addChild(p);
    }

    return false;
}

void TitleScene::onTouchMoved(Touch* touch, Event* unused_event) {
    CCLOG("onTouchMoved");
}

void TitleScene::onTouchEnded(Touch* touch, Event* unused_event) {
    CCLOG("onTouchEnded");
}

void TitleScene::onTouchCancelled(Touch* touch, Event* unused_event) {
    CCLOG("onTouchCancelled");
}

//背景の初期化
void TitleScene::initTitle() {
    // タイトル画像
    auto title = Sprite::create("Title.png");
    title->setPosition(WINSIZE / 2);
    addChild(title);

}

void TitleScene::initMenu(){
    auto label1 = Label::createWithSystemFont("スタート", "fonts/Marker Felt.ttf", 64);
    label1->enableShadow(Color4B::GRAY, Size(0.5, 0.5), 15);
    label1->enableOutline(Color4B::BLUE, 5);
    auto item1 = MenuItemLabel::create(label1, [&](Ref* pSender) {
        //指定秒数後に次のシーンへ
        nextScene(STAGE_SELECT_SCENE, 1.0f);
    });

    auto menu = Menu::create(item1, NULL);
    menu->alignItemsVerticallyWithPadding(50);
    this->addChild(menu);
}

//次のシーンへ遷移
void TitleScene::nextScene(int NEXT_SCENE, float dt) {
    // 次のシーンを生成する
    Scene* scene;
    switch(NEXT_SCENE){
        case STAGE_SELECT_SCENE:
            scene = StageSelectScene::createScene();
            break;
        default:
            scene = NULL;
    }

    Director::getInstance()->replaceScene(
            TransitionFadeTR::create(dt, scene)
    );
}
