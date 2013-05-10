#ifndef BLOCK_H
#define BLOCK_H

#include "common.h"

WISP_NAMESPACE_BEGIN

#define WISP_BLOCK_SIZE 32

class ImageBlock
{
	const static int FILTER_TABLE_SIZE = 32;
public:
    ImageBlock(const Vector2i& size, const Filter* filter);
    ~ImageBlock();

    inline void setOffset(const Point2i& offset) { m_offset = offset; }
    inline const Point2i& getOffset() const { return m_offset; }
    inline void setSize(const Vector2i& size) { m_size = size; }
    inline void setSize(int x, int y) { m_size.x = x; m_size.y = y; }
    inline const Vector2i& getSize() const { return m_size; }
    inline const Vector2i& getTotalSize() const { return m_totalSize; }
    inline const Vector2i& getBorderSize() const { return m_borderSize; }
    void clear();

    const Color4f* getData() const { return m_results; }
    bool put(const Point2f& pos, const Color3f& value);
    bool put(ImageBlock& block);

    inline void lock() const { m_mutex.lock(); }
    inline void unlock() const { m_mutex.unlock(); }
    std::string toString() const;

private:
    const Color4f& get(int x, int y) const;

protected:
    Point2i m_offset;
    Vector2i m_size;
    Vector2i m_borderSize;
    Vector2i m_totalSize;
    Color4f* m_pixels;
    Color4f* m_results;
    Vector2f m_filterSize;
    float* m_filterTable;
	int* m_ifx;
	int* m_ify;
    mutable std::mutex m_mutex;
};

class BlockGenerator
{
public:
    BlockGenerator(const Vector2i& size, int blockSize);
    bool next(ImageBlock& block);

protected:
    enum EDirection { ERight, EDown, ELeft, EUp};

    Point2i m_block;
    Vector2i m_numBlocks;
    Vector2i m_size;
    int m_blockSize;
    int m_numSteps;
    int m_blocksLeft;
    int m_stepsLeft;
    int m_direction;
    std::mutex m_mutex;
};

class BlockRenderThread
{
public:
    BlockRenderThread(const Scene* scene, Sampler* sampler,
                      BlockGenerator* blockGenerator, ImageBlock* output, Timer* timer);

    void operator()();

private:
    const Scene* m_scene;
    BlockGenerator* m_blockGenerator;
    ImageBlock* m_output;
    Sampler* m_sampler;
    Timer* m_timer;
};

WISP_NAMESPACE_END
#endif // BLOCK_H
