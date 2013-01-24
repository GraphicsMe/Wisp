#include "../gui/mainwindow.h"
#include "scene.h"
#include "block.h"
#include "camera.h"
#include "parser.h"

#include <iostream>

#include <QDebug>
#include <QApplication>

using namespace Wisp;

void render(Scene* scene, std::string strFileName)
{
    const Camera* camera = scene->getCamera();
    Vector2i outputSize = camera->getOutputSize();
    ImageBlock result(outputSize, camera->getFilter());

    MainWindow mainWindow(&result);
    mainWindow.show();
    BlockGenerator blockGenerator(outputSize, WISP_BLOCK_SIZE);
    int nCores = getCoreCount();
    std::vector<std::thread*> threads;
	
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
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "App path : " << qApp->applicationDirPath();
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: Wisp <scene.xml>" << std::endl;
            return -1;
        }
        Wisp::Object* root = Wisp::loadScene(argv[1]);
        if (root->getClassType() == Wisp::Object::EScene)
        {
            Wisp::Scene* scene = static_cast<Wisp::Scene*>(root);
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
