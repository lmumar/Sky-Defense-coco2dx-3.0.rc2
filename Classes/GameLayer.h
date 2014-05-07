#ifndef __GAME_LAYER_H__
#define __GAME_LAYER_H__

#include "cocos2d.h"

USING_NS_CC;

enum {
    kBackground,
    kMiddleground,
    kForeground
};

enum {
    kSpriteHalo,
    kSpriteSparkle,
    kSpriteMeteor,
    kSpriteHealth
};

class GameLayer : public cocos2d::Layer
{
private:
    int     _healthPoolIndex = 0;
    int     _meteorPoolIndex = 0;
    int     _shockWaveHits   = 0;
    int     _score           = 0;
    int     _energy          = 0;
    bool    _running         = false;
    
    int     _meteorSpeed;
    float   _meteorInterval;
    float   _meteorTimer;
    
    float   _healthInterval;
    float   _healthTimer;
    float   _healthSpeed;
    
    float   _difficultyInterval;
    float   _difficultyTimer;

    Size    _screenSize;
    Label  *_scoreDisplay;
    Label  *_energyDisplay;
    Sprite *_bomb;
    Sprite *_shockWave;
    Sprite *_introMessage;
    Sprite *_gameOverMessage;
    
    RepeatForever   *_swingHealth;
    Sequence        *_shockWaveSequence;
    Sequence        *_groundHit;
    Sequence        *_explosion;
    ScaleTo         *_growBomb;
    ActionInterval  *_rotateSprite;

    Vector<Sprite *> _clouds;
    Vector<Sprite *> _meteorPool;
    Vector<Sprite *> _healthPool;

    void createGameScreen();
    void createPools();
    void createActions();
    void resetGame();
    void stopGame();

    void shockWaveDone(Node *sender);
    void animationDone(Node *sender);
    
    void onTouchesBegan(const std::vector<Touch *>& touches, Event *unused_event);

public:
    static cocos2d::Scene* createScene();

    virtual bool init() override;
    void update(float delta);
    
    CREATE_FUNC(GameLayer);
};

#endif // __GAME_LAYER_H__
