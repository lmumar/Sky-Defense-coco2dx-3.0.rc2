#include "AppDelegate.h"
#include "GameLayer.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

using namespace CocosDenshion;

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
        glview = GLView::create("Sky Defense");
        director->setOpenGLView(glview);
    }

    // initialize audio engine
    auto bgmusic = FileUtils::getInstance()->fullPathForFilename("background.mp3");
    SimpleAudioEngine::getInstance()->preloadBackgroundMusic(bgmusic.c_str());
    SimpleAudioEngine::getInstance()->setEffectsVolume(0.4f);

    Size screenSize = glview->getFrameSize();
    Size designSize{2048, 1536};
    glview->setDesignResolutionSize
        (designSize.width, designSize.height, ResolutionPolicy::EXACT_FIT);
    
    if (screenSize.height > 768) {
        FileUtils::getInstance()->setSearchPaths({"ipadhd"});
    } else if (screenSize.height > 320) {
        FileUtils::getInstance()->setSearchPaths({"ipad"});
    } else {
        FileUtils::getInstance()->setSearchPaths({"iphone"});
    }

    director->setContentScaleFactor(screenSize.height/designSize.height);

    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);

    // create a scene. it's an autorelease object
    auto scene = GameLayer::createScene();

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();
    SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();
    SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
