#include "HitObject.h"
#include "Constants.h"
#include "GameScene.h"

namespace yuzu
{
    // x*(gScreenWidth-100)/512+50 will scale the x position from osu!pixels to pixels, centered
    HitObject::HitObject(int x, int w, int h, int t, SDL_Texture *texture) :
            Sprite(static_cast<int>(x * (constants::gScreenWidth - 100.0) / 512.0 + 50), START_Y, w, h, texture), time(t) {}

    void HitObject::start()
    {
        startTime = SDL_GetTicks64();
        state = HitObjectState::ACTIVE;
        catcher = GameScene::getInstance()->getCatcher();
    }

    void HitObject::update()
    {
        Sprite::update();

        auto gameScene = GameScene::getInstance();

        // move the hit object down
        SDL_Rect rect = getRect();
        // the speed is calculated from DROP_TIME, which is the milliseconds it takes to fall from START_Y to HIT_Y
        rect.y = static_cast<int>(START_Y + (HIT_Y - START_Y) * (SDL_GetTicks64() - startTime) / DROP_TIME);
        setRect(rect);

        // check if the hit object is out of hit bounds
        if ((rect.y + rect.h) > MISS_Y && state == HitObjectState::ACTIVE)
        {
            state = HitObjectState::MISSED;

            gameScene->score->processHitObject(this);
        }

        // check if the hit object is in hit bounds
        // since rhythm games are very precise, we can not use collision detection here
        if ((rect.y + rect.h) > HIT_Y && state == HitObjectState::ACTIVE)
        {
            SDL_Point plateRange = catcher->getPlateRange(); // (x, x2)

            // the fruit only needs to be partially in the plate range to be hit, not fully
            if (rect.x + rect.w > plateRange.x && rect.x < plateRange.y)
            {
                state = HitObjectState::HIT;
                gameScene->score->processHitObject(this);
                gameScene->removeComponent(this, true);
            }
        }

        // remove & destroy if out of the screen
        if (rect.y > constants::gScreenHeight)
        {
            state = HitObjectState::DESTROYED;
            gameScene->removeComponent(this, true);
        }
    }

} // yuzu