/*
 * NParticleSystemQuad.h
 *
 *  Created on: 2015/02/19
 *      Author: Chara
 */

#ifndef __NPARTICLESYSTEMQUAD_H_
#define __NPARTICLESYSTEMQUAD_H_

#include "cocos2d.h"

class NParticleSystemQuad : public cocos2d::ParticleSystemQuad {

public:
    static NParticleSystemQuad* create(const std::string& filename);
    virtual void update(float dt) override;
    std::function<void(NParticleSystemQuad*)> onFinishListener;
};

#endif /* __NPARTICLESYSTEMQUAD_H_ */
