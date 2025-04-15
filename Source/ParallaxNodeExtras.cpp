#include "ParallaxNodeExtras.h"
#include "2d/ParallaxNode.cpp"

// Need to provide a constructor
ParallaxNodeExtras::ParallaxNodeExtras()
{
    ParallaxNode();  // call parent constructor
}

ParallaxNodeExtras* ParallaxNodeExtras::node()
{
    return new ParallaxNodeExtras();
}

void ParallaxNodeExtras::incrementOffset(Point offset, Node* node)
{
    for (auto i : _parallaxArray)
    {
        PointObject* point = (PointObject*)i;
        Node* currentNode = point->getChild();
        if (currentNode == node)
        {
            point->setOffset(point->getOffset() + offset);
            break;
        }
    }
}
