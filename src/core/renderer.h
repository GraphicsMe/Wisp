#ifndef RENDER_H
#define RENDER_H

#include "common.h"

WISP_NAMESPACE_BEGIN

void render(Scene* scene, const std::string& strFileName)
{
    const Camera* camera = scene->getCamera();
    Vector2i outputSize = camera->getOutputSize();

    BlockGenerator blockGenerator(outputSize, WISP_BLOCK_SIZE);

    int nCores = getCoreCount();
    std::vector<BlockRenderThread*> threads;
}

WISP_NAMESPACE_END

#endif // RENDER_H
