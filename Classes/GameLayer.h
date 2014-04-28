#ifndef __GAME_LAYER_H__
#define __GAME_LAYER_H__

#include "cocos2d.h"

USING_NS_CC;

enum {
    kBackground,
    kMiddleground,
    kForeground
};

class GameLayer : public cocos2d::Layer
{
private:
    Size _screenSize;

    void createGameScreen();

public:
    static cocos2d::Scene* createScene();

    virtual bool init() override;
    
    CREATE_FUNC(GameLayer);
};

#endif // __GAME_LAYER_H__
