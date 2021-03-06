#include "CommonGameBase.h"
#include "Utils.h"

namespace oreore
{
    using namespace ::cocos2d;

    /* IDConnector */
    unsigned int IDConnector::id = 0;

    /* ScreenBase */
    ScreenBase::ScreenBase()
    {
        _scene = Scene::create();
        _scene->retain();
        _scene->addChild(this);
    }

    ScreenBase::~ScreenBase()
    {
        _scene->release();
    }

    SpriteBatchNode *ScreenBase::registerSprite(const char *filename)
    {
        SpriteBatchNode *batch = SpriteBatchNode::create(filename);
        addChild(batch);
        return batch;
    }
}
