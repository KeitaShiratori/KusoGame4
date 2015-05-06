#ifndef __KusoGame1__EnemySprite__
#define __KusoGame1__EnemySprite__

#include "cocos2d.h"
USING_NS_CC;

class EnemySprite : public cocos2d::Sprite
{
public:
    EnemySprite(); //コンストラクタ
    static EnemySprite* create(std::string filePath); //インスタンス生成
    virtual bool init(std::string filePath); //初期化
    static EnemySprite* createWithTexture(Texture2D *texture);
    virtual bool initWithTexture(Texture2D *texture);

    Vec2 _velocity;
    float _absVelocity;
    Sprite* _velocityVector;
    float _len;
    float _angle;
    int  _type;
    int _charge;
    int _mass;
    bool _isMyMedal;
    bool _isDead;

    cocos2d::CCRect getRect();
    bool isTouchPoint(cocos2d::CCPoint);
    void calculateVelocity(Vector<EnemySprite*> vMedal); // 速度ベクトルを計算する。O(n)
    void setAbsVelocity(float absVelocity); // 速度ベクトルの絶対値を設定。O(1)
    float getAbsVelocity(); // 速度ベクトルの絶対値を返す。O(1)
    void setVelocity(Vec2 velocity); // 速度ベクトルを設定。O(1)
    Vec2 getVelocity(); // 速度ベクトルを返す。O(1)
    bool isDead();
    void isDead(bool isDead);
    bool isMyMedal();
    void isMyMedal(bool isMyMedal);
    void setPType();
    void setNType();
    int  getType();
    void setCharge(int charge);
    int getCharge();
    void setMass(int mass);
    int getMass();
    void describe();
    void showVector();

};

#endif /* defined(__KusoGame1__EnemySprite__) */
