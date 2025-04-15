#ifndef ParallaxNodeExtras_h
#define ParallaxNodeExtras_h

#include "axmol.h"

USING_NS_AX;

class ParallaxNodeExtras : public ParallaxNode
{

public:
    // Need to provide a constructor
    ParallaxNodeExtras();

    // just to avoid ugly later cast and also for safety
    static ParallaxNodeExtras* node();

    // Facility method (it’s expected to have it soon in COCOS2DX)
    void incrementOffset(Point offset, Node* node);
};

#endif
