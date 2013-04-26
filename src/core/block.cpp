#include "block.h"
#include "sampler.h"
#include "scene.h"
#include "integrator.h"
#include "camera.h"
#include "filter.h"


WISP_NAMESPACE_BEGIN

ImageBlock::ImageBlock(const Vector2i& size, const Filter* filter)
    : m_size(size)
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
        m_ifx[x-x0] = std::min((int)std::floor(fx), FILTER_TABLE_SIZE - 1);
    }

    float invFilterY = 1.0f / m_filterSize.y;
    for (int y = y0; y <= y1; ++y)
    {
        float fy = std::abs(y - posY) * invFilterY * FILTER_TABLE_SIZE;
        m_ify[y-y0] = std::min((int)std::floor(fy), FILTER_TABLE_SIZE - 1);
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
	Vector2i realOff = offset + m_borderSize - block.getBorderSize();
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

std::string ImageBlock::toString() const
{
    return formatString("ImageBlock[offset=%d %d, size=%d, %d]",
                        m_offset.x, m_offset.y,
                        m_size.x, m_size.y);
}

BlockGenerator::BlockGenerator(const Vector2i& size, int blockSize)
    : m_size(size), m_blockSize(blockSize)
{
    m_numBlocks = Vector2i(
                (int)std::ceil(size.x / blockSize),
                (int)std::ceil(size.y / blockSize));
    m_blocksLeft = m_numBlocks.x * m_numBlocks.y;
    m_direction = ERight;
    m_block = glm::max(Point2i(0, 0), Point2i(m_numBlocks/2-1));
    m_stepsLeft = 1;
    m_numSteps = 1;
}

bool BlockGenerator::next(ImageBlock& block)
{
    m_mutex.lock();
    if (m_blocksLeft == 0)
    {
        m_mutex.unlock();
        return false;
    }

    Point2i pos = m_block * m_blockSize; //pixel coordinate

    // set block parameters
    block.setOffset(pos);
    block.setSize(glm::min(m_size-pos, m_blockSize));

    if (--m_blocksLeft == 0)
    {
        std::cout << "Rendering finished!" << std::endl;
        m_mutex.unlock();
        return true;
    }

    do
    {
        switch(m_direction)
        {
        case ERight:    ++m_block.x; break;
        case EDown:     ++m_block.y; break;
        case ELeft:     --m_block.x; break;
        case EUp:       --m_block.y; break;
        }
        if (--m_stepsLeft == 0) // change direction
        {
            m_direction = (m_direction + 1) % 4;
            if (m_direction == ELeft || m_direction == ERight)
                ++m_numSteps;
            m_stepsLeft = m_numSteps;
        }
    }while (m_block.x < 0 || m_block.y < 0 ||
            m_block.x >= m_numBlocks.x ||
            m_block.y >= m_numBlocks.y);

    m_mutex.unlock();
    return true;
}

BlockRenderThread::BlockRenderThread(const Scene *scene, Sampler *sampler,
    BlockGenerator *blockGenerator, ImageBlock *output)
    : m_scene(scene)
    , m_blockGenerator(blockGenerator)
    , m_output(output)
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
                    for (size_t i = 0; i < m_sampler->getSampleCount(); ++i)
                    {
                        Point2f pixelSample = Point2f(offset.x + x, offset.y + y)
                                + m_sampler->next2D();
                        Point2f lensSample = m_sampler->next2D();
                        TRay ray;
                        float weight = camera->generateRay(pixelSample, lensSample, ray);
                        Color3f radiance = integrator->Li(m_scene, m_sampler, ray);

                        block.put(pixelSample, weight*radiance);
                    }
                }
            }
            m_output->put(block);
        }
    }
    catch(const WispException& ex)
    {
        std::cerr << "Caught a critical exception within a rendering thread: " << ex.getReason() << std::endl;
        exit(-1);
    }
}

WISP_NAMESPACE_END
