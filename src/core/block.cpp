#include "block.h"
#include "sampler.h"
#include "scene.h"
#include "integrator.h"
#include "camera.h"
#include "filter.h"
#include "timer.h"


WISP_NAMESPACE_BEGIN

std::atomic<double> g_renderFinishTime = 0;

ImageBlock::ImageBlock(const Vector2i& size, const Filter* filter)
    : m_size(size), m_offset(0)
{
    m_filterSize = filter->getFilterSize();
    m_borderSize = Vector2i(std::ceil(m_filterSize.x - 0.5f), std::ceil(m_filterSize.y - 0.5f));
    m_totalSize = m_size + 2*m_borderSize;

    int totalSize = m_totalSize.x * m_totalSize.y;
    m_pixels = new Color4f[totalSize];
    m_results = new Color4f[totalSize];
    memset(m_pixels, 0, sizeof(Color4f) * totalSize);
    memset(m_results, 0, sizeof(Color4f) * totalSize);

    m_filterTable = new float[FILTER_TABLE_SIZE * FILTER_TABLE_SIZE];
    float* ftp = m_filterTable;
    for (int y = 0; y < FILTER_TABLE_SIZE; ++y)
    {
        float fy = (y + 0.5f) * m_filterSize.y / FILTER_TABLE_SIZE;
        for (int x = 0; x < FILTER_TABLE_SIZE; ++x)
        {
            float fx = (x + 0.5f) * m_filterSize.x / FILTER_TABLE_SIZE;
            *ftp++ = filter->evaluate(fx, fy);
        }
    }
	int xc = (int)std::ceil(2.0f * m_filterSize.x + 1);
	int yc = (int)std::ceil(2.0f * m_filterSize.y + 1);
	m_ifx = new int[xc];
	m_ify = new int[yc];
	memset(m_ifx, 0, sizeof(int) * xc);
	memset(m_ify, 0, sizeof(int) * yc);
}

ImageBlock::~ImageBlock()
{
    delete[] m_ifx;
	delete[] m_ify;
	delete[] m_pixels;
    delete[] m_results;
    delete[] m_filterTable;
}

void ImageBlock::clear()
{
    int totalSize = m_totalSize.x * m_totalSize.y;
    memset(m_pixels, 0, sizeof(Color4f) * totalSize);
    memset(m_results, 0, sizeof(Color4f) * totalSize);
}

bool ImageBlock::put(const Point2f& pos, const Color3f& value)
{
    assert (isValid(value));
    float posX = pos.x - 0.5f + m_borderSize.x - m_offset.x;
    float posY = pos.y - 0.5f + m_borderSize.y - m_offset.y;
    int x0 = (int)std::ceil(posX - m_filterSize.x);
    int y0 = (int)std::ceil(posY - m_filterSize.y);
    int x1 = (int)std::floor(posX + m_filterSize.x);
    int y1 = (int)std::floor(posY + m_filterSize.y);
    x0 = clamp(x0, 0, m_totalSize.x-1);
    x1 = clamp(x1, 0, m_totalSize.x-1);
    y0 = clamp(y0, 0, m_totalSize.y-1);
    y1 = clamp(y1, 0, m_totalSize.y-1);

    float invFilterX = 1.0f / m_filterSize.x;
    for (int x = x0; x <= x1; ++x)
    {
        float fx = std::abs(x - posX) * invFilterX * FILTER_TABLE_SIZE;
        m_ifx[x-x0] = min((int)std::floor(fx), FILTER_TABLE_SIZE - 1);
    }

    float invFilterY = 1.0f / m_filterSize.y;
    for (int y = y0; y <= y1; ++y)
    {
        float fy = std::abs(y - posY) * invFilterY * FILTER_TABLE_SIZE;
        m_ify[y-y0] = min((int)std::floor(fy), FILTER_TABLE_SIZE - 1);
    }

    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            int offset = m_ify[y-y0] * FILTER_TABLE_SIZE + m_ifx[x-x0];
			assert (offset >= 0 && offset < FILTER_TABLE_SIZE*FILTER_TABLE_SIZE);

            float weight = m_filterTable[offset];

            int id = m_totalSize.x * y + x;
            assert (id >= 0 && id < m_totalSize.x * m_totalSize.y);
            m_pixels[id] += weight * Color4f(value.x, value.y, value.z, 1.0f);
            m_results[id] = m_pixels[id] / m_pixels[id].w;
        }
    }

	return true;
}

bool ImageBlock::put(ImageBlock& block)
{
    this->lock();
    Vector2i offset = block.getOffset();
    Vector2i realOff = offset + m_borderSize + m_offset - block.getBorderSize();
    Vector2i size = block.getTotalSize();
    
    for (int j = 0; j < size.y; ++j)
    {
		for (int i = 0; i < size.x; ++i)
		{
			int x = i + realOff.x;
			int y = j + realOff.y;
			int id = m_totalSize.x * y + x;
			assert (id >= 0 && id < m_totalSize.x*m_totalSize.y);
			m_pixels[id] += block.get(i, j);
			m_results[id] = m_pixels[id] / m_pixels[id].w;
		}
    }

    this->unlock();
    return true;
}

const Color4f& ImageBlock::get(int x, int y) const
{
	int id = y*m_totalSize.x + x;
	assert (id >= 0 && id < m_totalSize.x*m_totalSize.y);
    return m_pixels[id];
}

bool ImageBlock::savePfm(const char* filename)
{
    FILE* f;
    errno_t err=fopen_s(&f, filename, "wb");
    if(err != 0)
        return false;

    fprintf_s(f, "PF\n%d %d\n%f\n", m_size.x, m_size.y, -1.f);
    Color4f* data = m_results + m_totalSize.x * (m_borderSize.y+m_size.y-1) + m_borderSize.x;
    float* copy = new float[m_size.x * m_size.y * 3];
    float* temp = copy;
    for (int i = 0; i < m_size.y; ++i)
    {
        for (int j = 0; j < m_size.x; ++j)
        {
            *temp++ = data[j].x;
            *temp++ = data[j].y;
            *temp++ = data[j].z;
        }
        data -= m_size.x;
    }
    int n = fwrite(copy,sizeof(float)*3, m_size.x * m_size.y, f);
    assert (n == m_size.x * m_size.y);

    delete [] copy;
    fclose(f);
    return true;
}

std::string ImageBlock::toString() const
{
    return formatString("ImageBlock[offset=%d %d, size=%d, %d]",
                        m_offset.x, m_offset.y,
                        m_size.x, m_size.y);
}

BlockGenerator::BlockGenerator(const Vector2i& size, int blockSize)
{
    Vector2i numBlocks = Vector2i((int)std::ceil(size.x / blockSize),
                                  (int)std::ceil(size.y / blockSize));
    m_blocksNumberTotal = numBlocks.x * numBlocks.y;
    m_curBlockIndex = 0;

    enum EDirection { ERight, EDown, ELeft, EUp};

    int numSteps = 1;
    int stepsLeft = 1;
    int direction = ERight;
    Point2i curBlock = Point2i(max(0, numBlocks.x/2-1), max(0, numBlocks.y/2-1));


    for (int i = 0; i < m_blocksNumberTotal; ++i)
    {
        Vector2i off = curBlock * blockSize;
        Vector2i curSize(min(size.x-off.x, blockSize), min(size.y-off.y, blockSize));
        m_precomputedBlock.push_back(std::make_pair(off, curSize));

        if (i == m_blocksNumberTotal - 1)
            break;
        do
        {
            switch(direction)
            {
            case ERight:    ++curBlock.x; break;
            case EDown:     ++curBlock.y; break;
            case ELeft:     --curBlock.x; break;
            case EUp:       --curBlock.y; break;
            }
            if (--stepsLeft == 0) // change direction
            {
                direction = (direction + 1) % 4;
                if (direction == ELeft || direction == ERight)
                    ++numSteps;
                stepsLeft = numSteps;
            }
        }while (curBlock.x < 0 || curBlock.y < 0 ||
                curBlock.x >= numBlocks.x ||
                curBlock.y >= numBlocks.y);
    }
}

bool BlockGenerator::next(ImageBlock& block)
{
    if (m_curBlockIndex == m_blocksNumberTotal)
        return false;

    OffsetSize offSize = m_precomputedBlock[m_curBlockIndex++];
    block.setOffset(offSize.first);
    block.setSize(offSize.second);
    if (m_curBlockIndex == m_blocksNumberTotal)
        std::cout << "Rendering finished!" << std::endl;
    return true;
}

BlockRenderThread::BlockRenderThread(const Scene *scene, Sampler *sampler,
    BlockGenerator *blockGenerator, ImageBlock *output, Timer* timer)
    : m_scene(scene)
    , m_blockGenerator(blockGenerator)
    , m_output(output)
    , m_timer(timer)
{
    m_sampler = sampler->clone();
}

void BlockRenderThread::operator()()
{
    std::chrono::milliseconds dura( 500 );
    std::this_thread::sleep_for( dura );
    try
    {
        const Camera* camera = m_scene->getCamera();
        const Integrator* integrator = m_scene->getIntegrator();

        ImageBlock block(Vector2i(WISP_BLOCK_SIZE), camera->getFilter());

        while (m_blockGenerator->next(block))
        {
            if (!m_scene->isRendering())
                break;
            Point2i offset = block.getOffset();
            Vector2i size = block.getSize();

            block.clear();
            for (int y = 0; y < size.y; ++y)
            {
                for (int x = 0; x < size.x; ++x)
                {
                    m_sampler->generate();
                    for (size_t i = 0; i < m_sampler->getSampleCount(); ++i)
                    {
                        Point2f pixelSample = Point2f(offset.x + x, offset.y + y)
                                + m_sampler->next2D();
                        Point2f lensSample = m_sampler->next2D();
                        Ray ray;
                        float weight = camera->generateRay(pixelSample, lensSample, ray);
                        Color3f radiance = integrator->Li(m_scene, m_sampler, ray);
                        block.put(pixelSample, weight*radiance);
                        m_sampler->advance();
                    }
                }
            }
            m_output->put(block);
        }
        double time = m_timer->currentTime();
        double t = g_renderFinishTime.load();
        if (time > t)
            g_renderFinishTime.store(time);
    }
    catch(const WispException& ex)
    {
        std::cerr << "Caught a critical exception within a rendering thread: " << ex.getReason() << std::endl;
        exit(-1);
    }
}

WISP_NAMESPACE_END
