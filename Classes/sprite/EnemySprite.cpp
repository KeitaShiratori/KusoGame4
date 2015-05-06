#include "EnemySprite.h"

USING_NS_CC;

#define UNIT_LEN 80000 //8000 * 8000 * 100

//コンストラクタ
EnemySprite::EnemySprite() :
        _absVelocity(0),
        _velocityVector(nullptr),
        _len(0),
        _angle(0),
        _isDead(false),
        _isMyMedal(false),
        _type(0),
        _charge(1),
        _mass(1)
{
    _velocity = Vec2(0.0f, 0.0f);
}

//インスタンス生成
EnemySprite* EnemySprite::create(std::string filePath)
{
    EnemySprite *pRet = new EnemySprite();
    if (pRet && pRet->init(filePath)) {
        pRet->autorelease();
        return pRet;
    }
    else {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

//初期化
bool EnemySprite::init(std::string filePath) {
    if (!Sprite::initWithFile(filePath))
        return false;

    return true;
}

EnemySprite* EnemySprite::createWithTexture(Texture2D *texture)
{
    EnemySprite *sprite = new (std::nothrow) EnemySprite();
    if (sprite && sprite->initWithTexture(texture))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

bool EnemySprite::initWithTexture(Texture2D *texture)
{
    CCASSERT(texture != nullptr, "Invalid texture for sprite");

    Rect rect = Rect::ZERO;
    rect.size = texture->getContentSize();

    return Sprite::initWithTexture(texture, rect);
}

/**
 *  スプライトの範囲を取得
 *
 *  @return CCRect
 */
CCRect EnemySprite::getRect()
{
    // スプライトの座標（画像の真ん中の座標のこと）
    CCPoint point = this->getPosition();

    // スプライトの幅と高さ
    int w = this->getContentSize().width;
    int h = this->getContentSize().height;

    // スプライトの範囲を返す
    return CCRectMake(point.x-(w/2), point.y-(h/2), w, h);
}

/**
 *  引数の座標がスプライトの範囲内かどうかを返す（タッチ時の座標情報を渡す）
 *
 *  @param point 座標ポイント
 *
 *  @return ture or false
 */
bool EnemySprite::isTouchPoint(CCPoint point)
{
    return this->getRect().containsPoint(point);
}

void EnemySprite::calculateVelocity(Vector<EnemySprite*> vMedal){
    _velocity = _velocity / 1.1f;

    // 速度ベクトルを計算する。O(n)
    for(auto medal : vMedal){
        if(medal->isDead()){
            continue;
        }

        float x = medal->getPositionX() - this->getPositionX();
        float y = medal->getPositionY() - this->getPositionY();
        float l = x * x + y * y;

        if(l == 0){
            continue;
        }

        // 方向を調整。絶対値で割って、単位ベクトルにする。
        if(x != 0)
            x = -1 * x * medal->getType() * this->getType() / fabsf(x);

        if(y != 0)
            y = -1 * y * medal->getType() * this->getType() / fabsf(y);

        // 速度ベクトルの補正（電荷と重さと定数）
        x = 256 * x * sqrt(sqrt(medal->getCharge() * this->getCharge())) / sqrt(sqrt(this->getMass()));
        y = 256 * y * sqrt(sqrt(medal->getCharge() * this->getCharge())) / sqrt(sqrt(this->getMass()));

        _velocity += Vec2(x * x * x / sqrt(l), y * y * y / sqrt(l));
    }

    _absVelocity = sqrt(_velocity.x * _velocity.x + _velocity.y * _velocity.y);

    showVector();
}

void EnemySprite::setAbsVelocity(float absVelocity){
    _absVelocity = absVelocity;
}

float EnemySprite::getAbsVelocity(){
    return _absVelocity; // 速度ベクトルの絶対値を返す。O(1)
}

void EnemySprite::setVelocity(Vec2 velocity){
    _velocity = velocity;
}

Vec2 EnemySprite::getVelocity(){
    return _velocity; // 速度ベクトルを返す。O(1)
}

bool EnemySprite::isDead(){
    return _isDead;
}

void EnemySprite::isDead(bool isDead){
    _isDead = isDead;
}

bool EnemySprite::isMyMedal(){
    return _isMyMedal;
}

void EnemySprite::isMyMedal(bool isMyMedal){
    _isMyMedal = isMyMedal;
}

void EnemySprite::setPType(){
    _type = 1;
}

void EnemySprite::setNType(){
    _type = -1;
}

int EnemySprite::getType(){
    return _type;
}
void EnemySprite::setCharge(int charge){
    _charge = charge;
}
int EnemySprite::getCharge(){
    return _charge;
}

void EnemySprite::setMass(int mass){
    _mass = mass;
}

int EnemySprite::getMass(){
    return _mass;
}

void EnemySprite::describe(){
    // 価数の表示
    auto chargeLabel = Label::createWithSystemFont(StringUtils::format(" %i", _charge), "fonts/Marker Felt.ttf", 48);
    chargeLabel->enableShadow(Color4B::WHITE, Size(0.5, 0.5), 5);
    chargeLabel->enableOutline(Color4B::WHITE, 2);
    chargeLabel->setPosition(Vec2(getContentSize().width * 0.4f, getContentSize().height * 0.7));
    this->addChild(chargeLabel, 2);

    // 符号の表示
    std::string strType = "";
    if(_type > 0){
        strType = "+";
    }
    else if(_type < 0){
        strType = "-";
    }

    auto typeLabel = Label::createWithSystemFont(strType, "fonts/Marker Felt.ttf", 48);
    typeLabel->enableShadow(Color4B::WHITE, Size(0.5, 0.5), 5);
    typeLabel->enableOutline(Color4B::WHITE, 2);
    typeLabel->setPosition(Vec2(getContentSize().width * 0.6f, getContentSize().height * 0.7));
    this->addChild(typeLabel, 2);

    // 質量の表示
    auto massLabel = Label::createWithSystemFont(StringUtils::format("%i", _mass), "fonts/Marker Felt.ttf", 48);
    massLabel->enableShadow(Color4B::WHITE, Size(0.5, 0.5), 5);
    massLabel->enableOutline(Color4B::WHITE, 2);
    massLabel->setPosition(Vec2(getContentSize().width * 0.5f, getContentSize().height * 0.3));
    this->addChild(massLabel, 2);

}

void EnemySprite::showVector(){
    if(_velocityVector == nullptr){
        _velocityVector = Sprite::create("vector2.png");
        _velocityVector->setPosition(getContentSize() / 2);

        _velocityVector->runAction(RepeatForever::create(
                            Sequence::create(
                                    FadeTo::create(1.0f, 176),
                                    FadeTo::create(1.0f, 244),
                                    nullptr)));

        this->addChild(_velocityVector);
    }

    CCLOG("_absVelocity: %f", _absVelocity);
    CCLOG("UNIT_LEN: %d", UNIT_LEN);
    CCLOG("_absVelocity / UNIT_LEN: %f", _absVelocity / UNIT_LEN);

    _len = _absVelocity;
    if(_len < (UNIT_LEN * 0.001)){
        _len = 0.0f;
    }
    else if(_len < (UNIT_LEN * 0.4)){
        _len = 0.4f;
    }
    else if(_len > (UNIT_LEN * 3.6)){
        _len = 1.2f;
    }
    else{
        _len = 0.4f + (_len / UNIT_LEN / 4) - 0.1f;
    }

    CCLOG("_len: %f", _len);

    _velocityVector->setScale(_len);
    _angle = ccpToAngle(_velocity)*-180/M_PI;
    _velocityVector->setRotation(_angle + 90.0f);
}
