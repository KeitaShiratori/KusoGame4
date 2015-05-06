#include "StageSelectScene.h"
#include "QuestScene.h"
#include "TitleScene.h"
#include "platform/NativeBridge.h"
#include "utility/UIDialog.h"
#include "stdlib.h"
#include "extensions/cocos-ext.h"

#define WINSIZE Director::getInstance()->getWinSize()
#define T_DIALOG 1000
#define QUEST_SCENE 1

USING_NS_CC_EXT;
USING_NS_CC;

//コンストラクタ
StageSelectScene::StageSelectScene()
{

}

//シーン生成
Scene* StageSelectScene::createScene() {
    auto scene = Scene::create();
    auto layer = StageSelectScene::create();
    scene->addChild(layer);
    return scene;
}

//インスタンス生成
StageSelectScene* StageSelectScene::create() {
    StageSelectScene *pRet = new StageSelectScene();
    pRet->init();
    pRet->autorelease();

    return pRet;
}

//初期化
bool StageSelectScene::init() {
    if (!Layer::init())
        return false;

    // パスを追加
    CCFileUtils::sharedFileUtils()->addSearchPath("extensions");

    // 湯気エフェクトのプール
    _pool = ParticleSystemPool::create("yuge_texture.plist", 20);
    _pool->retain();
    _clearPool = ParticleSystemPool::create("clearIcon_texture.plist", 50);
    _clearPool->retain();

    // シングルタップイベントの取得
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(_swallowsTouches);
    touchListener->onTouchBegan = CC_CALLBACK_2(StageSelectScene::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(StageSelectScene::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(StageSelectScene::onTouchEnded, this);
    touchListener->onTouchCancelled = CC_CALLBACK_2(StageSelectScene::onTouchCancelled, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    initBackground(); //タイトルの初期化
    initMenu();

    return true;
}

bool StageSelectScene::onTouchBegan(Touch* touch, Event* unused_event) {
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

void StageSelectScene::onTouchMoved(Touch* touch, Event* unused_event) {
    CCLOG("onTouchMoved");
}

void StageSelectScene::onTouchEnded(Touch* touch, Event* unused_event) {
    CCLOG("onTouchEnded");
}

void StageSelectScene::onTouchCancelled(Touch* touch, Event* unused_event) {
    CCLOG("onTouchCancelled");
}

//背景の初期化
void StageSelectScene::initBackground() {
    // 背景画像
    auto title = Sprite::create("StageSelect.png");
    title->setPosition(WINSIZE / 2);
    addChild(title);

    auto label1 = Label::createWithSystemFont("タイトル画面へ", "fonts/Marker Felt.ttf", 48);
    label1->enableShadow(Color4B::GRAY, Size(0.5, 0.5), 5);
    label1->enableOutline(Color4B::BLUE, 1);
    auto item1 = MenuItemLabel::create(label1, [&](Ref* pSender) {
        //指定秒数後に次のシーンへ
        auto scene = TitleScene::createScene();
        Director::getInstance()->replaceScene(
                TransitionFade::create(1.0f, scene, Color3B::BLACK)
        );
    });

    auto menu = Menu::create(item1, NULL);
    menu->setPosition(Vec2(WINSIZE.width * 0.15, WINSIZE.height * 0.03));
    this->addChild(menu, ZOrder::Background);
}

void StageSelectScene::initMenu(){
    // ステージクリア情報保存先の生成
    UserDefault *userDef = UserDefault::getInstance();
    bool isAllClear = true;
    for(int row = 0; row < 10; row++){
        auto menu = Menu::create(NULL);

        for(int i = 1; i <= 5; i++){
            int level = i + 5 * row;

            std::string keyIsOpen = StringUtils::format("IS_OPEN_STAGE%2d", level);
            bool isOpen = userDef->getBoolForKey(keyIsOpen.c_str(), false);

            std::string keyIsClear = StringUtils::format("IS_CLEAR_STAGE%2d", level);
            bool isClear = userDef->getBoolForKey(keyIsClear.c_str(), false);
            isAllClear *= isClear;

            if(isOpen){
                // 解放されているステージを表示
                auto item = MenuItemFont::create(StringUtils::format("[%2d]", level), [&, level](Object* pSender) {
                    // 処理
                    nextScene(QUEST_SCENE, level, 1.0f);
                });

                if(isClear){
                    Sprite* clearIcon = Sprite::create("stageClearIcon.png");
                    clearIcon->setPosition(item->getContentSize());
                    item->addChild(clearIcon, ZOrder::Icon);

                    auto p = _clearPool->pop();
                    if(p != nullptr){
                        p->setPosition(item->getContentSize());
                        p->setAutoRemoveOnFinish(true); // 表示が終わったら自分を親から削除！
                        item->addChild(p, ZOrder::Icon);
                    }
                }
                item->setFontSizeObj(64);
                menu->addChild(item, ZOrder::Label);
            }
        }

        menu->setPosition(WINSIZE.width * 0.5, WINSIZE.height * (0.88 - 0.07 * row));
        menu->alignItemsHorizontallyWithPadding(50);
        this->addChild(menu, ZOrder::Background);
    }

    if(isAllClear){
        // TODO
        // 全ステージクリアした場合の演出
    }
}

//次のシーンへ遷移
void StageSelectScene::nextScene(int NEXT_SCENE, int level, float dt) {
    // 次のシーンを生成する
    Scene* scene;
    switch(NEXT_SCENE){
        case QUEST_SCENE:
            scene = QuestScene::createScene(level, 0.0f);
            break;
        default:
            scene = NULL;
    }

    Director::getInstance()->replaceScene(
            TransitionFadeTR::create(dt, scene)
    );
}
