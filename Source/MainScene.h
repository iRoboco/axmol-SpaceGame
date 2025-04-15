#ifndef __MAIN_SCENE_H__
#define __MAIN_SCENE_H__

#include "axmol.h"
#include "ParallaxNodeExtras.h"

USING_NS_AX;

enum class EndReason
{
    WIN,
    LOSE
};

enum class Tag
{
    SHIP,
    LASER
};

class MainScene : public Scene
{
    enum class GameState
    {
        init = 0,
        update,
        pause,
        end,
        menu1,
        menu2,
    };

public:
    NodeGrid* _gameNode;
    bool init() override;
    bool onPhysicsContact(PhysicsContact& contact);
    void update(float delta) override;

    // touch
    void onTouchesBegan(const std::vector<Touch*>& touches, Event* event);
    void onTouchesMoved(const std::vector<Touch*>& touches, Event* event);
    void onTouchesEnded(const std::vector<Touch*>& touches, Event* event);

    // mouse
    // void onMouseDown(Event* event);
    // void onMouseUp(Event* event);
    // void onMouseMove(Event* event);
    // void onMouseScroll(Event* event);

    // Keyboard
    void onKeyPressed(EventKeyboard::KeyCode code, Event* event);
    void onKeyReleased(EventKeyboard::KeyCode code, Event* event);

    // a selector callback
    void menuCloseCallback(Object* sender);

    SpriteBatchNode* _batchNode;
    Sprite* _ship;

    float randomValueBetween(float low, float high);
    void setInvisible(Node* node);
    float getTimeTick();

    static constexpr auto ASTEROIDS = 50;
    static constexpr auto LASERS    = 15;

    void laserShooting();
    ParticleExplosion* makeExplosion(Object* sender, Vec2& explosionPos);

private:
    GameState _gameState = GameState::init;

    EventKeyboard* touchListener;
    EventKeyboard* keyboardListener;

    ParallaxNodeExtras* _backgroundNode;
    Sprite* _spacedust1;
    Sprite* _spacedust2;
    Sprite* _planetsunrise;
    Sprite* _galaxy;
    Sprite* _spacialanomaly;
    Sprite* _spacialanomaly2;

    float _shipPointsPerSecY;
    Vec2 _shipStartPosition;

    Vector<Sprite*>* _asteroids;
    int _nextAsteroid;
    float _nextAsteroidSpawn;

    Vector<Sprite*>* _shipLasers;
    int _nextShipLaser;

    int _lives;

    double _gameOverTime;
    bool _gameOver;
    void endScene(EndReason endReason);
    void restartTapped();

    std::chrono::steady_clock::time_point _timerStart;
    ActionInterval* _repeatLaserShooting;
};

#endif  // __MAIN_SCENE_H__
