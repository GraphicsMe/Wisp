#include "../gui/mainwindow.h"
#include "scene.h"
#include "block.h"
#include "camera.h"
#include "parser.h"

#include <iostream>

#include <QDebug>
#include <QApplication>
#include "common.h"

using namespace Wisp;

Scene* Wisp::g_scene;
std::string Wisp::g_sceneDirectory;

void render(Scene* scene, std::string strFileName)
{
    const Camera* camera = scene->getCamera();
    Vector2i outputSize = camera->getOutputSize();
    ImageBlock result(outputSize, camera->getFilter());

    MainWindow mainWindow(&result, scene);
    mainWindow.show();
    BlockGenerator blockGenerator(outputSize, WISP_BLOCK_SIZE);

    scene->start();

    // 1. multi-threaded
    std::vector<std::thread*> threads;
    int nCores = getCoreCount();
    for (int i = 0; i < nCores; ++i)
    {
        threads.push_back(new std::thread(BlockRenderThread(scene,
            scene->getSampler(), &blockGenerator, &result)));
    }

    mainWindow.startRefresh();
    qApp->exec();
    mainWindow.stopRefresh();

    for (size_t i = 0; i < threads.size(); ++i)
    {
        threads[i]->join();
        delete  threads[i];
    }

    /*
    // 2. single-threaded
    BlockRenderThread th(scene,
                scene->getSampler(), &blockGenerator, &result);
    th();*/
}

void extractSceneDirectory(const std::string& sceneFile)
{
    int pos = sceneFile.find_last_of('\\');
    if (pos != std::string::npos)
        g_sceneDirectory = sceneFile.substr(0, pos+1);
    else
    {
        pos = sceneFile.find_last_of('\/');
        assert (pos != std::string::npos);
        g_sceneDirectory = sceneFile.substr(0, pos+1);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: Wisp <scene.xml>" << std::endl;
            return -1;
        }
        extractSceneDirectory(argv[1]);
        Wisp::Object* root = Wisp::loadScene(argv[1]);
        if (root->getClassType() == Wisp::Object::EScene)
        {
            Wisp::Scene* scene = static_cast<Wisp::Scene*>(root);
            g_scene = scene;
            render(scene, "wisp.pfm");
        }
    }
    catch(const Wisp::WispException& ex)
    {
        std::cerr << "Caught a critical exception: " << ex.getReason() << std::endl;
        return -1;
    }

    return 0;
}
