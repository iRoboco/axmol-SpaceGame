#include "MainScene.h"
#include "audio/AudioEngine.h"

USING_NS_AX;

bool MainScene::init()
{
    //////////////////////////////
    // Super init first
    if (!Scene::initWithPhysics()) {
        return false;
    }

    auto winSize = _director->getWinSize();
    auto visibleSize = _director->getVisibleSize();
    auto origin = _director->getVisibleOrigin();
    auto safeArea = _director->getSafeAreaRect();
    auto safeOrigin = safeArea.origin;

    Rect gridRect = Rect(Vec2::ZERO, visibleSize);
    _gameNode = NodeGrid::create(gridRect);
    this->addChild(_gameNode);

    /////////////////////////////
    // Setup physics
    getPhysicsWorld()->setGravity(Vec2(0, 0));
    getPhysicsWorld()->setSlopBias(0, 0);
    getPhysicsWorld()->setSubsteps(360);
    //getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = AX_CALLBACK_1(MainScene::onPhysicsContact, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

    /////////////////////////////
    // Create menu
    auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
        AX_CALLBACK_1(MainScene::menuCloseCallback, this));
    float x = safeOrigin.x + safeArea.size.width - closeItem->getContentSize().width / 2;
    float y = safeOrigin.y + closeItem->getContentSize().height / 2;
    closeItem->setPosition(Vec2(x, y));
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // Input listeners
    auto touchListener = EventListenerTouchAllAtOnce::create();
    touchListener->onTouchesBegan = AX_CALLBACK_2(MainScene::onTouchesBegan, this);
    touchListener->onTouchesMoved = AX_CALLBACK_2(MainScene::onTouchesMoved, this);
    touchListener->onTouchesEnded = AX_CALLBACK_2(MainScene::onTouchesEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    // auto mouseListener           = EventListenerMouse::create();
    // mouseListener->onMouseMove   = AX_CALLBACK_1(MainScene::onMouseMove, this);
    // mouseListener->onMouseUp     = AX_CALLBACK_1(MainScene::onMouseUp, this);
    // mouseListener->onMouseDown   = AX_CALLBACK_1(MainScene::onMouseDown, this);
    // mouseListener->onMouseScroll = AX_CALLBACK_1(MainScene::onMouseScroll, this);
    //_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = AX_CALLBACK_2(MainScene::onKeyPressed, this);
    keyboardListener->onKeyReleased = AX_CALLBACK_2(MainScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithFixedPriority(keyboardListener, 11);

    /////////////////////////////
    // Space and Ship
    _batchNode = SpriteBatchNode::create("Spritesheets/Sprites.pvr.ccz");
    _gameNode->addChild(_batchNode);
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("Spritesheets/Sprites.plist");

    _ship = Sprite::createWithSpriteFrameName("SpaceFlier_sm_1.png");
    auto shipBody = PhysicsBody::createCircle(_ship->getContentSize().width / 5, PhysicsMaterial(1.0f, 0.0f, 0.0f));
    shipBody->setPositionOffset(Vec2(16, 0));
    shipBody->setContactTestBitmask(1);
    _ship->addComponent(shipBody);

    _shipStartPosition = Vec2(winSize.width / 10, winSize.height / 2);
    _ship->setPosition(_shipStartPosition);
    _batchNode->addChild(_ship, 2, (int)Tag::SHIP);

    // Create the ParallaxNode
    _backgroundNode = ParallaxNodeExtras::node();
    _gameNode->addChild(_backgroundNode, -2);
    auto shaky = Shaky3D::create(1.0f, Size(5.0f, 5.0f), 1.0f, false);
    _gameNode->runAction(shaky);


    // Create the sprites will be added to the ParallaxNode
    _spacedust1 = Sprite::create("Backgrounds/bg_front_spacedust.png");
    _spacedust2 = Sprite::create("Backgrounds/bg_front_spacedust.png");
    _planetsunrise = Sprite::create("Backgrounds/bg_planetsunrise.png");
    _galaxy = Sprite::create("Backgrounds/bg_galaxy.png");
    _spacialanomaly = Sprite::create("Backgrounds/bg_spacialanomaly.png");
    _spacialanomaly2 = Sprite::create("Backgrounds/bg_spacialanomaly2.png");

    // Determine relative movement speeds for space dust and background
    Point dustSpeed = Vec2(0.1, 0.1);
    Point bgSpeed = Vec2(0.05, 0.05);

    // Add children to ParallaxNode
    _backgroundNode->addChild(_spacedust1, 0, dustSpeed, Vec2(0, winSize.height / 2));  // 2
    _backgroundNode->addChild(_spacedust2, 0, dustSpeed, Vec2(_spacedust1->getContentSize().width, winSize.height / 2));
    _backgroundNode->addChild(_galaxy, -1, bgSpeed, Vec2(0, winSize.height * 0.7));
    _backgroundNode->addChild(_planetsunrise, -1, bgSpeed, Vec2(600, winSize.height * 0));
    _backgroundNode->addChild(_spacialanomaly, -1, bgSpeed, Vec2(900, winSize.height * 0.3));
    _backgroundNode->addChild(_spacialanomaly2, -1, bgSpeed, Vec2(1500, winSize.height * 0.9));

    /////////////////////////////
    // Stars
    _gameNode->addChild(ParticleSystemQuad::create("Particles/Stars1.plist"), -1);
    _gameNode->addChild(ParticleSystemQuad::create("Particles/Stars2.plist"), -1);
    _gameNode->addChild(ParticleSystemQuad::create("Particles/Stars3.plist"), -1);

    /////////////////////////////
    // Asteroids
    _asteroids = new Vector<Sprite*>(ASTEROIDS);
    for (int i = 0; i < ASTEROIDS; ++i) {
        auto asteroid = Sprite::createWithSpriteFrameName("asteroid.png");
        asteroid->setVisible(false);
        _batchNode->addChild(asteroid, 1);
        _asteroids->pushBack(asteroid);
    }
    _nextAsteroidSpawn = 0;

    /////////////////////////////
    // Lalers
    _shipLasers = new Vector<Sprite*>(LASERS);
    for (int i = 0; i < LASERS; ++i) {
        auto shipLaser = Sprite::createWithSpriteFrameName("laserbeam_blue.png");
        shipLaser->setVisible(false);
        _batchNode->addChild(shipLaser, 1, (int)Tag::LASER);
        _shipLasers->pushBack(shipLaser);
    }

    /////////////////////////////
    // Win/Lose detection
    _lives = 3;
    double curTime = getTimeTick();
    _gameOverTime = curTime + 30000;

    /////////////////////////////
    // Audio background effect
    AudioEngine::play2d("sounds/SpaceGame.wav", true);

    /////////////////////////////
    // scheduleUpdate() is required to ensure update(float) is called on every loop
    scheduleUpdate();

    return true;
}

void MainScene::onTouchesBegan(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    for (auto&& t : touches) {
        AXLOG("onTouchesBegan detected, X:%f  Y:%f", t->getLocation().x, t->getLocation().y);
        laserShooting();
    }
}

void MainScene::onTouchesMoved(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    for (auto&& t : touches) {
        AXLOG("onTouchesMoved detected, X:%f  Y:%f", t->getLocation().x, t->getLocation().y);
    }
}

void MainScene::onTouchesEnded(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    for (auto&& t : touches) {
        AXLOG("onTouchesEnded detected, X:%f  Y:%f", t->getLocation().x, t->getLocation().y);
    }
}

// void MainScene::onMouseDown(Event* event)
//{
//     EventMouse* e = static_cast<EventMouse*>(event);
//     AXLOG("onMouseDown detected, Key: %d", static_cast<int>(e->getMouseButton()));
// }
//
// void MainScene::onMouseUp(Event* event)
//{
//     EventMouse* e = static_cast<EventMouse*>(event);
//     AXLOG("onMouseUp detected, Key: %d", static_cast<int>(e->getMouseButton()));
// }
//
// void MainScene::onMouseMove(Event* event)
//{
//     EventMouse* e = static_cast<EventMouse*>(event);
//     AXLOG("onMouseMove detected, X:%f  Y:%f", e->getCursorX(), e->getCursorY());
// }
//
// void MainScene::onMouseScroll(Event* event)
//{
//     EventMouse* e = static_cast<EventMouse*>(event);
//     AXLOG("onMouseScroll detected, X:%f  Y:%f", e->getScrollX(), e->getScrollY());
// }

void MainScene::onKeyPressed(EventKeyboard::KeyCode code, Event* event)
{
    AXLOG("onKeyPressed, keycode: %d", static_cast<int>(code));
    switch (code) {
    case EventKeyboard::KeyCode::KEY_UP_ARROW:
    case EventKeyboard::KeyCode::KEY_W:
        _shipPointsPerSecY = 300;
        break;
    case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
    case EventKeyboard::KeyCode::KEY_S:
        _shipPointsPerSecY = -300;
        break;
    case EventKeyboard::KeyCode::KEY_SPACE:
        laserShooting();
        break;
    }
}

void MainScene::onKeyReleased(EventKeyboard::KeyCode code, Event* event)
{
    AXLOG("onKeyReleased, keycode: %d", static_cast<int>(code));
    switch (code) {
    case EventKeyboard::KeyCode::KEY_UP_ARROW:
    case EventKeyboard::KeyCode::KEY_W:
        _shipPointsPerSecY = 0;
        break;
    case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
    case EventKeyboard::KeyCode::KEY_S:
        _shipPointsPerSecY = 0;
        break;
    case EventKeyboard::KeyCode::KEY_SPACE:
        break;
    }
}

void MainScene::update(float delta)
{
    /////////////////////////////
    // Background moving
    Point backgroundScrollVert = Vec2(-1000, 0);
    Point bgPosition = _backgroundNode->getPosition();
    bgPosition.add(backgroundScrollVert * delta);
    _backgroundNode->setPosition(bgPosition);

    Vector<Sprite*> spaceDusts{ _spacedust1, _spacedust2 };

    for (auto ii : spaceDusts) {
        Sprite* spaceDust = ii;
        float xPosition = _backgroundNode->convertToWorldSpace(spaceDust->getPosition()).x;
        float size = spaceDust->getContentSize().width;
        if (xPosition < -size / 2) {
            _backgroundNode->incrementOffset(Vec2(spaceDust->getContentSize().width * 2, 0), spaceDust);
        }
    }

    Vector<Sprite*> backGrounds{ _galaxy, _planetsunrise, _spacialanomaly, _spacialanomaly2 };
    for (auto ii : backGrounds) {
        Sprite* background = ii;
        float xPosition = _backgroundNode->convertToWorldSpace(background->getPosition()).x;
        float size = background->getContentSize().width;
        if (xPosition < -size) {
            _backgroundNode->incrementOffset(Vec2(2000, 0), background);
        }
    }

    if (!_gameOver) {
        /////////////////////////////
        // Spaceship mooving
        Size winSize = Director::getInstance()->getWinSize();
        float maxY = winSize.height - _ship->getContentSize().height / 2;
        float minY = _ship->getContentSize().height / 2;

        float diff = (_shipPointsPerSecY * delta);
        float newY = _ship->getPosition().y + diff;
        newY = MIN(MAX(newY, minY), maxY);
        _ship->setPosition(Vec2(_ship->getPosition().x, newY));

        /////////////////////////////
        // Asteroids mooving
        float curTimeMillis = getTimeTick();
        if (curTimeMillis > _nextAsteroidSpawn) {

            float randMillisecs = randomValueBetween(0.20f, 1.0f) * 1000;
            _nextAsteroidSpawn = randMillisecs + curTimeMillis;

            float randY = randomValueBetween(0.0f, winSize.height);
            float randDuration = randomValueBetween(1.0f, 3.0f);
            float randAngel = randomValueBetween(-150.0f, 150.0f);

            auto asteroid = _asteroids->at(_nextAsteroid);
            _nextAsteroid++;

            if (_nextAsteroid >= _asteroids->size())
                _nextAsteroid = 0;

            asteroid->stopAllActions();
            asteroid->setPosition(winSize.width + asteroid->getContentSize().width / 2, randY);
            auto asteroidBody =
                PhysicsBody::createCircle(asteroid->getContentSize().width / 2.7f, PhysicsMaterial(10.0f, 0.0f, 0.0f));
            asteroidBody->setContactTestBitmask(1);
            asteroid->addComponent(asteroidBody);
            asteroid->setVisible(true);
            asteroid->runAction(Sequence::create(
                MoveBy::create(randDuration, Vec2(-winSize.width - asteroid->getContentSize().width, 0)),
                CallFuncN::create(AX_CALLBACK_1(MainScene::setInvisible, this)), nullptr));
            asteroid->runAction(RepeatForever::create(RotateBy::create(randDuration / 2, randAngel)));
        }

        /////////////////////////////
        // Remove physial bodies over screen
        for (auto asteroid : *_asteroids) {
            // Remove asteroid physical body over ship
            if (asteroid->getPosition().x < winSize.width * 0.1) {
                asteroid->removeAllComponents();
            }
            // Hide sprite over screen
            else if (asteroid->getPosition().x < -asteroid->getContentSize().width / 2) {
                asteroid->setVisible(false);
            }

            // Detect asteroid && laser collision
            if (!asteroid->isVisible())
                continue;
            for (auto& shipLaser : *_shipLasers) {
                // Remove laser physical body over screen
                if (shipLaser->getPosition().x > winSize.width - 5) {
                    shipLaser->removeAllComponents();
                }
            }
        }

        /////////////////////////////
        // Win/Loose detection
        if (_lives <= 0) {
            _ship->stopAllActions();
            _ship->setVisible(false);
            this->endScene(EndReason::LOSE);
        }
        else if (curTimeMillis >= _gameOverTime) {
            this->endScene(EndReason::WIN);
        }

        /////////////////////////////
        // Return ship to start position
        if (_ship->getPosition().x < winSize.x * 0.09) {
            auto offset = _shipStartPosition - _ship->getPosition();
            _ship->runAction(MoveBy::create(1.0f, Vec2(offset.width / 20, 0)));
        }
    }
}

bool MainScene::onPhysicsContact(PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();

    if (nodeA && nodeB) {
        AXLOG(">>> START collision detect...");
        for (auto asteroid : *_asteroids) {
            /////////////////////////////
            // Detect ship && asteroid collision
            if (_ship->getBoundingBox().intersectsRect(asteroid->getBoundingBox())) {
                if ((nodeA == _ship && nodeB == asteroid) || (nodeA == asteroid && nodeB == _ship)){
                    AXLOG(">>> Detect ship && asteroid collision");
                    asteroid->setVisible(false);
                    asteroid->removeAllComponents();
                    AudioEngine::play2d("sounds/explosion_large.wav");

                    auto explosionPos = asteroid->getPosition();
                    this->addChild(makeExplosion(this, explosionPos));
                    _lives--;

                    auto shaky = Shaky3D::create(1.2f, Size(10.0f, 10.0f), 5.0f, false);
                    _gameNode->runAction(shaky);
                }
            }

            /////////////////////////////
            // Detect laser && asteroid collision
            for (auto& shipLaser : *_shipLasers) {
                if (!shipLaser->isVisible())
                    continue;
                if (shipLaser->getBoundingBox().intersectsRect(asteroid->getBoundingBox())) {
                    if ((nodeA == shipLaser && nodeB == asteroid) || (nodeA == asteroid && nodeB == shipLaser)) {
                        shipLaser->setVisible(false);
                        shipLaser->removeAllComponents();

                        asteroid->setVisible(false);
                        asteroid->removeAllComponents();
                        AudioEngine::play2d("sounds/explosion_large.wav");

                        auto explosionPos = asteroid->getPosition();
                        this->addChild(makeExplosion(this, explosionPos));
                    }
                }
            }
        }
    }

    return true;
};

void MainScene::laserShooting()
{
    /////////////////////////////
    // Not shoting if gameover
    if (_gameOver)
        return;

    /////////////////////////////
    // Time delay for shoting
    auto timeElapsed = std::chrono::high_resolution_clock::now() - _timerStart;
    if (timeElapsed.count() < 150000000)
        return;

    /////////////////////////////
    // Laser object loop
    auto winSize = _director->getWinSize();
    Sprite* shipLaser = _shipLasers->at(_nextShipLaser++);
    if (_nextShipLaser >= _shipLasers->size())
        _nextShipLaser = 0;
    shipLaser->setPosition(Vec2(_ship->getPosition() + Vec2(shipLaser->getContentSize().width / 2, 0)));
    shipLaser->setVisible(true);

    auto laserBody = PhysicsBody::createCircle(5, PhysicsMaterial(1000.0f, 100.0f, 100.0f));
    laserBody->setPositionOffset(Vec2(16, 0));
    laserBody->setContactTestBitmask(1);
    shipLaser->addComponent(laserBody);

    shipLaser->stopAllActions();
    shipLaser->runAction(Sequence::create(MoveBy::create(0.5, Vec2(winSize.width, 0)),
        CallFuncN::create(AX_CALLBACK_1(MainScene::setInvisible, this)), nullptr));
    AudioEngine::play2d("sounds/laser_ship.wav");
    _timerStart = std::chrono::high_resolution_clock::now();
}

ParticleExplosion* MainScene::makeExplosion(Object* sender, Vec2& explosionPos)
{
    // auto explosion = ParticleSystem::create("Particles/sunExplosion.plist");
    auto explosion = ParticleExplosion::create();
    explosion->setPosition(explosionPos);
    explosion->setEmissionRate(500);
    explosion->setSpeed(300);
    return explosion;
}

void MainScene::endScene(EndReason endReason)
{
    if (_gameOver)
        return;
    _gameOver = true;

    /////////////////////////////
    // Remove all asteroids physical bodies
    _ship->removeAllComponents();
    for (auto asteroid : *_asteroids) {
        asteroid->removeAllComponents();
    }

    auto& winSize = _director->getWinSize();

    /////////////////////////////
    // Win or Loose message
    char message[10] = "You Win";
    if (endReason == EndReason::LOSE)
        strcpy(message, "You Lose");

    auto label = Label::createWithBMFont("Fonts/Arial.fnt", message);
    label->setScale(0.1);
    label->setPosition(winSize.width / 2, winSize.height * 0.6);
    this->addChild(label);

    /////////////////////////////
    // Restart button
    strcpy(message, "Restart");
    auto restartLabel = Label::createWithBMFont("Fonts/Arial.fnt", message);
    auto restartItem = MenuItemLabel::create(restartLabel, AX_CALLBACK_0(MainScene::restartTapped, this));
    restartItem->setScale(0.1f);
    restartItem->setPosition(winSize.width / 2, winSize.height * 0.4);

    auto menu = Menu::create(restartItem, nullptr);
    menu->setPosition(Point(0, 0));
    this->addChild(menu);

    /////////////////////////////
    // Clear label and menu
    restartItem->runAction(ScaleTo::create(0.5, 1.0));
    label->runAction(ScaleTo::create(0.5, 1.0));
}

float MainScene::randomValueBetween(float low, float high)
{
    return (rand_0_1() * (high - low)) + low;
}

float MainScene::getTimeTick()
{
    timeval time;
    gettimeofday(&time, NULL);
    unsigned long millisecs = (time.tv_sec * 1000) + (time.tv_usec / 1000);
    return (float)millisecs;
}

void MainScene::setInvisible(Node* node)
{
    node->setVisible(false);
}

void MainScene::restartTapped()
{
    /////////////////////////////
    // Terminate update callback
    this->unscheduleUpdate();

    /////////////////////////////
    // Stop all sounds
    AudioEngine::stopAll();

    /////////////////////////////
    // Scene recreate & transition
    auto scene = utils::createInstance<MainScene>();
    Director::getInstance()->replaceScene(TransitionZoomFlipX::create(0.5, scene));

    /////////////////////////////
    // Reschedule
    this->scheduleUpdate();
}

void MainScene::menuCloseCallback(ax::Object* sender)
{
    /////////////////////////////
    // Close the axmol game scene and quit the application
    _director->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use
     * _director->end() as given above,instead trigger a custom event created in RootViewController.mm
     * as below*/

     // EventCustom customEndEvent("game_scene_close_event");
     //_eventDispatcher->dispatchEvent(&customEndEvent);
}
