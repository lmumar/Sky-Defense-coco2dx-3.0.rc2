#include "GameLayer.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

using namespace CocosDenshion;

Scene* GameLayer::createScene()
{
    auto scene = Scene::create();
    auto layer = GameLayer::create();

    scene->addChild(layer);

    return scene;
}

bool GameLayer::init()
{
    if ( !Layer::init() ) {
        return false;
    }

    _screenSize = Director::getInstance()->getWinSize();

    createGameScreen();

    SimpleAudioEngine::getInstance()->playBackgroundMusic("background.mp3");

    return true;
}

void GameLayer::createGameScreen()
{
    auto bg = Sprite::create("bg.png");
    bg->setPosition(Point{
        _screenSize.width * 0.5f,
        _screenSize.height * 0.5f
    });
    addChild(bg);

    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("sprite_sheet.plist", "sprite_sheet.png");
    
    // create cityscape
    Sprite *sprite = nullptr;
    for (auto i = 0; i < 2; i++) {
        auto spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName("city_dark.png");
        sprite = Sprite::createWithSpriteFrame(spriteFrame);
        sprite->setPosition(Point{
            _screenSize.width * (0.25f + i * 0.5f),
            sprite->boundingBox().size.height * 0.5f
        });
        addChild(sprite, kForeground);

        spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName("city_light.png");
        sprite = Sprite::createWithSpriteFrame(spriteFrame);
        sprite->setPosition(Point{
            (0.25f + i * 0.5f),
            sprite->boundingBox().size.height * 0.9f
        });
        addChild(sprite, kBackground);
    }

    // add trees
    for (auto i = 0; i < 3; i++) {
        auto spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName("trees.png");
        sprite = Sprite::createWithSpriteFrame(spriteFrame);
        sprite->setPosition(Point{
            _screenSize.width * (0.2f + i * 0.3f),
            sprite->boundingBox().size.height * 0.5f
        });
        addChild(sprite, kForeground);
    }
}

