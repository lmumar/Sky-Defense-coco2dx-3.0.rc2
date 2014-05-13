#include "GameLayer.h"
#include "SimpleAudioEngine.h"


USING_NS_CC;

using namespace CocosDenshion;

GameLayer::~GameLayer() {
    CC_SAFE_RELEASE(_groundHit);
    CC_SAFE_RELEASE(_explosion);
    CC_SAFE_RELEASE(_growBomb);
    CC_SAFE_RELEASE(_rotateSprite);
}

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

    auto listener = EventListenerTouchAllAtOnce::create();
    listener->onTouchesBegan = CC_CALLBACK_2(GameLayer::onTouchesBegan, this);

    auto dispatcher = Director::getInstance()->getEventDispatcher();
    dispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    _screenSize = Director::getInstance()->getWinSize();

    createGameScreen();
    createPools();
    createActions();

    schedule(schedule_selector(GameLayer::update));

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
        sprite = Sprite::createWithSpriteFrameName("city_dark.png");
        sprite->setPosition(Point{
            _screenSize.width * (0.25f + i * 0.5f),
            sprite->boundingBox().size.height * 0.5f
        });
        addChild(sprite, kForeground);

        sprite = Sprite::createWithSpriteFrameName("city_light.png");
        sprite->setPosition(Point{
            (0.25f + i * 0.5f),
            sprite->boundingBox().size.height * 0.9f
        });
        addChild(sprite, kBackground);
    }

    // add trees
    for (auto i = 0; i < 3; i++) {
        sprite = Sprite::createWithSpriteFrameName("trees.png");
        sprite->setPosition(Point{
            _screenSize.width * (0.2f + i * 0.3f),
            sprite->boundingBox().size.height * 0.5f
        });
        addChild(sprite, kForeground);
    }

    // create labels
    _scoreDisplay = Label::createWithBMFont("font.fnt", "0");
    _scoreDisplay->setWidth(_screenSize.width * 0.3f);
    _scoreDisplay->setAnchorPoint(Point{1, 0.5});
    _scoreDisplay->setPosition(Point{_screenSize.width * 0.8f, _screenSize.height * 0.94f});
    addChild(_scoreDisplay);

    _energyDisplay = Label::createWithBMFont("font.fnt", "100%");
    _energyDisplay->setWidth(_screenSize.width * 0.1f);
    _energyDisplay->setHorizontalAlignment(TextHAlignment::RIGHT);
    _energyDisplay->setPosition(Point{_screenSize.width * 0.3f, _screenSize.height * 0.94f});
    addChild(_energyDisplay);
    
    auto iconFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName("health_icon.png");
    auto icon = Sprite::createWithSpriteFrame(iconFrame);
    icon->setPosition(Point{_screenSize.width * 0.15f, _screenSize.height * 0.94f});
    addChild(icon, kBackground);
    
    // create clouds
    float cloud_y;
    for (int i = 0; i < 4; i++) {
        cloud_y = i % 2 == 0 ? _screenSize.height * 0.4f : _screenSize.height * 0.5f;
        auto cloud = Sprite::createWithSpriteFrameName("cloud.png");
        cloud->setPosition(Point{_screenSize.width * 0.1f + i * _screenSize.width * 0.3f, cloud_y});
        addChild(cloud, kBackground);
        _clouds.pushBack(cloud);
    }
    
    // create bomb
    _bomb = Sprite::createWithSpriteFrameName("bomb.png");
    _bomb->getTexture()->generateMipmap();
    _bomb->setVisible(false);

    // add sparkle inside the bomb sprite
    Size size = _bomb->boundingBox().size;
    Sprite *sparkle = Sprite::createWithSpriteFrameName("sparkle.png");
    sparkle->setPosition(Point{
        size.width * 0.72f, size.height * 0.72f
    });
    _bomb->addChild(sparkle, kBackground, kSpriteSparkle);

    // add halo inside the bomb sprite
    Sprite *halo = Sprite::createWithSpriteFrameName("halo.png");
    halo->setPosition(Point{
        size.width * 0.4f, size.height * 0.4f
    });
    _bomb->addChild(halo, kMiddleground, kSpriteHalo);
    addChild(_bomb, kForeground);

    // create shock wave
    _shockWave = Sprite::createWithSpriteFrameName("shockwave.png");
    _shockWave->getTexture()->generateMipmap();
    _shockWave->setVisible(false);
    addChild(_shockWave);

    // create messages
    _introMessage = Sprite::createWithSpriteFrameName("logo.png");
    _introMessage->setPosition(Point{
        _screenSize.width * 0.5f, _screenSize.height * 0.6f
    });
    addChild(_introMessage, kForeground);

    _gameOverMessage = Sprite::createWithSpriteFrameName("gameover.png");
    _gameOverMessage->setPosition(Point{
        _screenSize.width * 0.5f, _screenSize.height * 0.65f
    });
    _gameOverMessage->setVisible(false);
    addChild(_gameOverMessage, kForeground);
}

void GameLayer::createPools() {
    for (int i = 0; i < 50; i++) {
        auto sprite = Sprite::createWithSpriteFrameName("meteor.png");
        sprite->setVisible(false);
        addChild(sprite, kMiddleground, kSpriteMeteor);
        _meteorPool.pushBack(sprite);
    }

    for (int i = 0; i < 20; i++) {
        auto sprite = Sprite::createWithSpriteFrameName("health.png");
        sprite->setVisible(false);
        addChild(sprite, kMiddleground, kSpriteHealth);
        _healthPool.pushBack(sprite);
    }
}

void GameLayer::createActions() {
    auto easeSwing = Sequence::create(
      EaseInOut::create(RotateTo::create(1.2f, -10), 2),
      EaseInOut::create(RotateTo::create(1.2f, 10), 2),
      nullptr
    );
    _swingHealth = RepeatForever::create(easeSwing);

    _growBomb = ScaleTo::create(6.0f, 1.0);
    _growBomb->retain();

    // action to rotate sprite
    auto rotate = RotateBy::create(0.5f, -90);
    _rotateSprite = RepeatForever::create(rotate);
    _rotateSprite->retain();

    Animation *animation = Animation::create();
    for (int i = 1; i <= 10; i++) {
        auto name = "boom" + std::to_string(i) + ".png";
        auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(name);
        animation->addSpriteFrame(frame);
    }
    animation->setDelayPerUnit(1/10);
    animation->setRestoreOriginalFrame(true);
    _groundHit = Sequence::create(MoveBy::create(0, Point{0, _screenSize.height * 0.12f}),
                                  Animate::create(animation),
                                  CallFuncN::create(std::bind(&GameLayer::animationDone, this, std::placeholders::_1)),
                                  nullptr);
    _groundHit->retain();

    animation = Animation::create();
    for (int i = 1; i <= 7; i++) {
        auto name  = "explosion_small" + std::to_string(i) + ".png";
        auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(name);
        animation->addSpriteFrame(frame);
    }
    animation->setDelayPerUnit(0.5/7.0);
    animation->setRestoreOriginalFrame(true);
    _explosion = Sequence::create(Animate::create(animation),
                                  CallFuncN::create(std::bind(&GameLayer::animationDone, this, std::placeholders::_1)),
                                  nullptr);
    _explosion->retain();
}

void GameLayer::shockWaveDone(Node *sender) {
    _shockWave->setVisible(false);
}

void GameLayer::animationDone(Node *sender) {
    sender->setVisible(false);
}

void GameLayer::onTouchesBegan(const std::vector<Touch *>& touches, Event *unused_event) {
    if (!_running) {
        if (_introMessage->isVisible()) {
            _introMessage->setVisible(false);
        } else if (_gameOverMessage->isVisible()) {
            SimpleAudioEngine::getInstance()->stopAllEffects();
            _gameOverMessage->setVisible(false);
        }

        resetGame();

        return;
    }

    Touch *touch = touches[0];

    if (touch) {
        if (_bomb->isVisible()) {
            // stop all actions in bomb, halo and sparkle
            _bomb->stopAllActions();
            Sprite *child = static_cast<Sprite *>(_bomb->getChildByTag(kSpriteHalo));
            child->stopAllActions();

            child = static_cast<Sprite *>(_bomb->getChildByTag(kSpriteSparkle));
            child->stopAllActions();

            // if bomb is the right scale, then create the shockwave
            if (_bomb->getScale() > 0.3f) {
                auto sequence = createShockWaveSequence();
                _shockWave->setScale(0.1f);
                _shockWave->setPosition(_bomb->getPosition());
                _shockWave->setVisible(true);
                _shockWave->runAction(ScaleTo::create(0.5f, _bomb->getScale() * 2.0f));
                _shockWave->runAction(sequence);
                SimpleAudioEngine::getInstance()->playEffect("bombRelease.wav");
            } else {
                SimpleAudioEngine::getInstance()->playEffect("bombFail.wav");
            }
            _bomb->setVisible(false);
            _shockWaveHits = 0;
        } else {
            // no bomb currently on screen, let's create one
            Point tap = touch->getLocation();
            _bomb->stopAllActions();
            _bomb->setScale(0.1f);
            _bomb->setPosition(tap);
            _bomb->setVisible(true);
            _bomb->setOpacity(50);
            _bomb->runAction(_growBomb);

            Sprite *child = static_cast<Sprite *>(_bomb->getChildByTag(kSpriteHalo));
            child->runAction(_rotateSprite);

            child = static_cast<Sprite *>(_bomb->getChildByTag(kSpriteSparkle));
            child->runAction(_rotateSprite);
        }
    }
}

void GameLayer::resetGame() {
    _score = 0;
    _energy = 100;
    
    
    // reset timers and speed
    _meteorInterval = 2.5f;
    _meteorTimer    = _meteorInterval * 0.99f;
    _meteorSpeed    = 10;
    
    _healthInterval = 20.0f;
    _healthTimer    = 0;
    _healthSpeed    = 15;
    
    _difficultyInterval = 60;
    _difficultyTimer    = 0;
    
    _running = true;
    
    auto value = std::to_string(_energy) + "%";
    _energyDisplay->setString(value);
    
    _scoreDisplay->setString(std::to_string(_score));
}

void GameLayer::stopGame() {
    _running = false;
    
    // stop all actions currently running
    if (_bomb->isVisible()) {
        _bomb->stopAllActions();
        _bomb->setVisible(false);
        Sprite *child = static_cast<Sprite *>(_bomb->getChildByTag(kSpriteHalo));
        child->stopAllActions();

        child = static_cast<Sprite *>(_bomb->getChildByTag(kSpriteSparkle));
        child->stopAllActions();
    }
    
    if (_shockWave->isVisible()) {
        _shockWave->stopAllActions();
        _shockWave->setVisible(false);
    }
}

void GameLayer::update(float delta) {
    if (!_running) {
        return;
    }

    _meteorTimer += delta;
    if (_meteorTimer > _meteorInterval) {
        _meteorTimer = 0;
        resetMeteor();
    }

    _healthTimer += delta;
    if (_healthTimer > _healthInterval) {
      _healthTimer = 0;
      resetHealth();
    }

    if (_bomb->isVisible()) {
      if (_bomb->getScale() > 0.3f) {
        if (_bomb->getOpacity() != 255) {
          _bomb->setOpacity(255);
        }
      }
    }

    // check collision with shockwave
    if (_shockWave->isVisible()) {
        float diffx, diffy;
        for (auto const& fallingObject : _fallingObjects) {
            diffx = _shockWave->getPositionX() - fallingObject->getPositionX();
            diffy = _shockWave->getPositionY() - fallingObject->getPositionY();
            if ((pow(diffx, 2) + pow(diffy, 2)) <= pow(_shockWave->boundingBox().size.width * 0.5f, 2)) {
                fallingObject->stopAllActions();
                fallingObject->runAction(_explosion->clone());
                SimpleAudioEngine::getInstance()->playEffect("boom.wav");
                if (fallingObject->getTag() == kSpriteMeteor) {
                  _shockWaveHits++;
                  _score += _shockWaveHits * 13 + _shockWaveHits * 2;
                }
                _fallingObjects.eraseObject(fallingObject);
            }
        }

        char szValue[100] = {0};
        std::sprintf(szValue, "%i", _score);
        _scoreDisplay->setString(szValue);
    }

    // move the clouds
    for (auto const& sprite : _clouds) {
        sprite->setPositionX(sprite->getPositionX() + delta * 20);
        if (sprite->getPositionX() > _screenSize.width + sprite->boundingBox().size.width * 0.5f) {
            sprite->setPositionX(-sprite->boundingBox().size.width * 0.5f);
        }
    }
}

void GameLayer::resetMeteor() {
    if (_fallingObjects.size() > 30) {
        return;
    }

    Sprite *meteor = _meteorPool.at(_meteorPoolIndex++);
    if (_meteorPoolIndex == _meteorPool.size()) {
        _meteorPoolIndex = 0;
    }

    int meteor_x = rand() % (int) (_screenSize.width * 0.8f) + _screenSize.width * 0.1f;
    int meteor_target_x = rand() % (int) (_screenSize.width * 0.8f) + _screenSize.width * 0.1f;

    meteor->stopAllActions();
    meteor->setPosition(Point{meteor_x, _screenSize.height + meteor->boundingBox().size.height * 0.5});

    auto rotate = RotateBy::create(0.5f, -90);
    auto rotateForever = RepeatForever::create(rotate);
    auto sequence = Sequence::create(MoveTo::create(_meteorSpeed, Point{meteor_target_x, _screenSize.height * 0.15f}),
                                     CallFuncN::create(
                                        std::bind(&GameLayer::fallingObjectDone, this, std::placeholders::_1)),
                                     nullptr);
    meteor->setVisible(true);
    meteor->runAction(rotateForever);
    meteor->runAction(sequence);

    _fallingObjects.pushBack(meteor);
}

void GameLayer::resetHealth() {
  if (_fallingObjects.size() > 30) {
    return;
  }

  Sprite *health = _healthPool.at(_healthPoolIndex++);
  if (_healthPoolIndex == _healthPool.size()) {
    _healthPoolIndex = 0;
  }

  int health_x = rand() % (int)(_screenSize.width * 0.8f) + _screenSize.width * 0.1f;
  int health_target_x = rand() % (int)(_screenSize.width * 0.8f) + _screenSize.width * 0.1f;

  health->stopAllActions();
  health->setPosition(Point{health_x, _screenSize.height + health->boundingBox().size.height * 0.5});

  auto sequence = Sequence::create(
    MoveTo::create(_healthSpeed, Point{health_target_x, _screenSize.height * 0.15f}),
    CallFuncN::create(
      std::bind(&GameLayer::fallingObjectDone, this, std::placeholders::_1)),
    nullptr
  );

  health->setVisible(true);
  health->runAction(sequence);
  _fallingObjects.pushBack(health);
}

void GameLayer::fallingObjectDone(cocos2d::Node *sender) {
    _fallingObjects.eraseObject(static_cast<Sprite *>(sender));
    sender->stopAllActions();
    sender->setRotation(0);

    if (sender->getTag() == kSpriteMeteor) {
        _energy -= 15;
        sender->runAction(_groundHit);
        SimpleAudioEngine::getInstance()->playEffect("boom.wav");
    } else {
        sender->setVisible(false);
        if (_energy == 100) {
          _score += 25;
          char score[100] = {0};
          std::sprintf(score, "%i", _score);
          _scoreDisplay->setString(score);
        } else {
          _energy += 10;
          if (_energy > 100) {
            _energy = 100;
          }
        }

        SimpleAudioEngine::getInstance()->playEffect("health.wav");
    }

    if (_energy <= 0) {
      _energy = 0;
      stopGame();
      SimpleAudioEngine::getInstance()->playEffect("fire_truck.wav");
      _gameOverMessage->setVisible(true);
    }

    char szValue[100] = {0};
    std::sprintf(szValue, "%i%s", _energy, "%");
    _energyDisplay->setString(szValue);
}

Sequence* GameLayer::createShockWaveSequence() {
    return Sequence::create(FadeOut::create(1.0f),
                            CallFuncN::create(
                              std::bind(&GameLayer::shockWaveDone, this, std::placeholders::_1)),
                            nullptr);
}

