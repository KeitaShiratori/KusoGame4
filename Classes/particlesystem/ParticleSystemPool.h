/*
 * ParticleSystemPool.h
 *
 *  Created on: 2015/02/19
 *      Author: Chara
 */

#ifndef __PARTICLESYSTEMPOOL_H_
#define __PARTICLESYSTEMPOOL_H_

#include "cocos2d.h"
#include "NParticleSystemQuad.h"

class ParticleSystemPool : public cocos2d::Ref {

public:
    static ParticleSystemPool* create(std::string filename, size_t size);
    bool init(std::string filename, size_t size);
    void push(NParticleSystemQuad* particle);
    NParticleSystemQuad* pop();

protected:
    /* プールの実体 */
    cocos2d::Vector<NParticleSystemQuad*> _pool;
};


#endif /* __PARTICLESYSTEMPOOL__H_ */
