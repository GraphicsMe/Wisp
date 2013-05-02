#include "../gui/mainwindow.h"
#include "scene.h"
#include "block.h"
#include "camera.h"
#include "parser.h"
#include "timer.h"

#include <fstream>
#include <iostream>

#include <QDebug>
#include <QApplication>
#include "common.h"

using namespace Wisp;

Wisp::Timer g_timer;
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
            scene->getSampler(), &blockGenerator, &result, &g_timer)));
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

void extractSceneDirectory(const std::string& sceneFile, std::string& fileName)
{
    int pos = sceneFile.find_last_of('\\');
    if (pos != std::string::npos)
    {
        fileName = sceneFile.substr(pos+1);
        g_sceneDirectory = sceneFile.substr(0, pos+1);
    }
    else
    {
        pos = sceneFile.find_last_of('/');
        assert (pos != std::string::npos);
        fileName = sceneFile.substr(pos+1);
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
        std::string fileName;
        extractSceneDirectory(argv[1], fileName);

        g_timer.start();
        Wisp::Object* root = Wisp::loadScene(argv[1]);
        double loadTime = g_timer.elapsedTime();
        double renderStart = g_timer.currentTime();
        if (root->getClassType() == Wisp::Object::EScene)
        {
            Wisp::Scene* scene = static_cast<Wisp::Scene*>(root);
            g_scene = scene;
            render(scene, "wisp.pfm");
        }
        double renderTime = g_renderFinishTime - renderStart;
        g_timer.stop();
        std::ofstream log("wisp.log", std::ofstream::app | std::ofstream::out);
        log << fileName << "\t\t" << std::fixed <<
               loadTime << "\t\t" << renderTime << std::endl;
    }
    catch(const Wisp::WispException& ex)
    {
        std::cerr << "Caught a critical exception: " << ex.getReason() << std::endl;
        return -1;
    }

    return 0;
}
