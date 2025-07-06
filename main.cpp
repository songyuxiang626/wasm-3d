#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <bx/readerwriter.h>
#include <bx/file.h>

#include <cstdint>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>

#include "./PlaneGeometry.cpp"
#include "./MeshBasicMaterial.cpp"
#include "./Utils.cpp"
#include "./Node.cpp"
#include "./Ray.cpp"
#include "bx/float4x4_t.h"
#include <set>

#include <bimg/bimg.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "common/common.h"
#include "common/font/font_manager.h"
#include "common/font/text_buffer_manager.h"
#include "common/font/text_metrics.h"
#include "common/cube_atlas.h"
#include "common/entry/entry.h"

// 全局变量
int screenWidth = 1280;
int screenHeight = 720;

float* view_matrix = nullptr;
float* projection_matrix = nullptr;
float viewInverse_matrix[16];
float projectionInverse_matrix[16];
float* textProjection_matrix = nullptr;

std::set<Node> nodes;

bgfx::TextureHandle textureHandle = BGFX_INVALID_HANDLE;
FontManager* m_fontManager = nullptr;
TextBufferManager* m_textBufferManager = nullptr;
TextBufferHandle m_scrollableBuffer = BGFX_INVALID_HANDLE;
TextBufferHandle m_textBuffer = BGFX_INVALID_HANDLE;
TextLineMetrics m_metrics = TextLineMetrics(FontInfo());
TrueTypeHandle m_font = BGFX_INVALID_HANDLE;
FontHandle m_fontSdf = BGFX_INVALID_HANDLE;
FontHandle m_fontScaled = BGFX_INVALID_HANDLE;
float m_textSize = 14.0f;
bx::StringView m_bigText;
const char* m_textBegin;
const char* m_textEnd;
const char* m_textBegin1;
const char* m_textEnd1;
float m_lineCount;

void downloadSucceed(emscripten_fetch_t * fetch) {
    stbi_set_flip_vertically_on_load(1);
    int width, height, channels;
    unsigned char* decodedData = stbi_load_from_memory(
        (const stbi_uc*)fetch->data, fetch->numBytes,
        &width, &height, &channels, 4
    );

    const bgfx::Memory* mem = bgfx::copy(decodedData, width * height * 4);
    textureHandle = bgfx::createTexture2D(
        width, height, false, 1, bgfx::TextureFormat::RGBA8, 0, mem
    );

    stbi_image_free(decodedData);

    emscripten_fetch_close(fetch);

	printf("loaded texture: %s, width: %d, height: %d, channels: %d\n",
		fetch->url, width, height, channels);
}

void downloadFailed(emscripten_fetch_t * fetch) {
    emscripten_fetch_close(fetch);
}

void renderFrame() {
    bgfx::touch(0);    

    bx::mtxInverse(viewInverse_matrix, view_matrix);
    bx::mtxInverse(projectionInverse_matrix, projection_matrix);
    bgfx::setViewTransform(1, view_matrix, textProjection_matrix);
    TextRectangle rectangle = m_textBufferManager->getRectangle(m_scrollableBuffer);
    float textAreaWidth = rectangle.width;
    
    float model_matrix[16];
    bx::mtxIdentity(model_matrix);
    float translation[16];
    bx::mtxTranslate(translation, -textAreaWidth*0.5f, -m_lineCount*m_metrics.getLineHeight()*0.5f, 0.0f);
    bx::mtxMul(model_matrix, model_matrix, translation);

    bgfx::setTransform(model_matrix);

    m_textBufferManager->submitTextBuffer(m_scrollableBuffer, 1);

    bgfx::setTransform(model_matrix);
    m_textBufferManager->submitTextBuffer(m_textBuffer, 1);

    if (bgfx::isValid(textureHandle)) {
        bgfx::setTexture(0, MeshBasicMaterial::u_texture, textureHandle);
    }

    bgfx::setViewTransform(0, view_matrix, projection_matrix);

	for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        Node node = *it;
        bgfx::setTransform(node.model_matrix);           

        bgfx::setVertexBuffer(0, node.vbh);
        bgfx::setIndexBuffer(node.ibh);
        bgfx::setState(BGFX_STATE_WRITE_RGB |
            BGFX_STATE_WRITE_A |
            BGFX_STATE_BLEND_ALPHA | 
            BGFX_STATE_CULL_CW);
        bgfx::submit(0, node.program);
	}

    bgfx::frame();
}

bx::Vec3 getWorldPositionByNdc(float x, float y) {
    float vec[4] = {x, y, 0.0f, 1.0f};
    float result[4];
    bx::vec4MulMtx(result, vec, projectionInverse_matrix);
    bx::vec4MulMtx(result, result, viewInverse_matrix);
	return bx::Vec3{result[0] / result[3], result[1] / result[3], 0.0f};
}

void intersectNode(float x, float y) {
	bx::Vec3 target = getWorldPositionByNdc(x, y);
    bool found = false;
    for (const Node& node : nodes) {
        for (int i = 0; i < 2; i++) { 
            float vec1[4] = {
                PlaneGeometry::PosTexCoord0Vertex[PlaneGeometry::indices[i * 3] * 5],
                PlaneGeometry::PosTexCoord0Vertex[PlaneGeometry::indices[i * 3] * 5 + 1],
                PlaneGeometry::PosTexCoord0Vertex[PlaneGeometry::indices[i * 3] * 5 + 2],
                1.0f
            };
            float result1[4];
            bx::vec4MulMtx(result1, vec1, node.model_matrix);
            bx::Vec3 v1 = {result1[0] / result1[3], result1[1] / result1[3], result1[2] / result1[3]};
            
            float vec2[4] = {
                PlaneGeometry::PosTexCoord0Vertex[PlaneGeometry::indices[i * 3 + 1] * 5],
                PlaneGeometry::PosTexCoord0Vertex[PlaneGeometry::indices[i * 3 + 1] * 5 + 1],
                PlaneGeometry::PosTexCoord0Vertex[PlaneGeometry::indices[i * 3 + 1] * 5 + 2],
                1.0f
            };
            float result2[4];
            bx::vec4MulMtx(result2, vec2, node.model_matrix);
            bx::Vec3 v2 = {result2[0] / result2[3], result2[1] / result2[3], result2[2] / result2[3]};
            
            float vec3[4] = {
                PlaneGeometry::PosTexCoord0Vertex[PlaneGeometry::indices[i * 3 + 2] * 5],
                PlaneGeometry::PosTexCoord0Vertex[PlaneGeometry::indices[i * 3 + 2] * 5 + 1],
                PlaneGeometry::PosTexCoord0Vertex[PlaneGeometry::indices[i * 3 + 2] * 5 + 2],
                1.0f
            };
            float result3[4];
            bx::vec4MulMtx(result3, vec3, node.model_matrix);
            bx::Vec3 v3 = {result3[0] / result3[3], result3[1] / result3[3], result3[2] / result3[3]};
            
			float u, v, t;
            found = Utils::rayIntersect(v1, v2, v3, Ray{{target.x, target.y, 5}, {0, 0, -1}}, u, v, t);
            if (found) {
                printf("Intersected: %d, u: %f, v: %f, t: %f\n", found, u, v, t);
                break;
            }
        }
        if (found) {
            break;
        }
    }
}

void onWindowResize(int width, int height) {
    screenWidth = width;
    screenHeight = height;

    bgfx::reset(screenWidth, screenHeight, BGFX_RESET_VSYNC);
    bgfx::setViewRect(0, 0, 0, screenWidth, screenHeight);
}

void addNodeByScreenXY(emscripten::val input_x_val, emscripten::val input_y_val, int order) {
	float input_x = input_x_val.as<float>();
	float input_y = input_y_val.as<float>();
	bx::Vec3 worldPosition = getWorldPositionByNdc(input_x, input_y);
	Node node(
        PlaneGeometry::vbh, 
        PlaneGeometry::ibh, 
        MeshBasicMaterial::program, 
        MeshBasicMaterial::u_texture);
	
	float identity[16];
	bx::mtxIdentity(identity);
	float translation[16];
	bx::mtxTranslate(translation, worldPosition.x, worldPosition.y, worldPosition.z);
	bx::mtxMul(node.model_matrix, identity, translation);
	
    node.order = order;
	nodes.insert(node);
}

void* load(const char* _filePath, uint32_t* _size)
{
    bx::AllocatorI* allocator = entry::getAllocator();  // ͳһʹ����� allocator
    bx::FileReader reader;

    if (bx::open(&reader, _filePath))
    {
        uint32_t size = (uint32_t)bx::getSize(&reader);

        if (size == 0)
        {
            printf("File %s has zero size!\n", _filePath);
            bx::close(&reader);
            if (_size != nullptr) *_size = 0;
            return nullptr;
        }

        void* data = bx::alloc(allocator, size);

        bx::read(&reader, data, size, bx::ErrorAssert{});
        bx::close(&reader);

        if (_size != nullptr)
        {
            *_size = size;
        }

        return data;
    }
    else
    {
        printf("Failed to open: %s\n", _filePath);
    }

    if (_size != nullptr)
    {
        *_size = 0;
    }

    return nullptr;
}

TrueTypeHandle loadTtf(FontManager* _fm, const char* _filePath)
{
    uint32_t size;
    void* data = load(_filePath, &size);

    if (nullptr != data)
    {
        TrueTypeHandle handle = _fm->createTtf((uint8_t*)data, size);

        bx::free(entry::getAllocator(), data);  // ��ͳһ allocator �ͷ�
        return handle;
    }

    TrueTypeHandle invalid = BGFX_INVALID_HANDLE;
    return invalid;
}

void initializeFont(uint32_t fontType)
{
    if (m_font.idx != bgfx::kInvalidHandle)
    {
        m_fontManager->destroyTtf(m_font);
    }

    if (m_fontScaled.idx != bgfx::kInvalidHandle)
    {
        m_fontManager->destroyFont(m_fontScaled);
    }
    if (m_fontSdf.idx != bgfx::kInvalidHandle)
    {
        m_fontManager->destroyFont(m_fontSdf);
    }

    if(m_textBufferManager != nullptr)delete m_textBufferManager;
    if(m_fontManager != nullptr)delete m_fontManager;    

    m_fontManager = new FontManager(512);
    m_textBufferManager = new TextBufferManager(m_fontManager);

    m_font = loadTtf(m_fontManager, "special_elite.ttf");

    m_fontSdf = m_fontManager->createFontByPixelSize(m_font, 0, 48, fontType, 6 + 2, 6 + 2);

    m_fontScaled = m_fontManager->createScaledFontToPixelSize(m_fontSdf, (uint32_t)m_textSize);

    m_metrics = TextLineMetrics(m_fontManager->getFontInfo(m_fontScaled));
}

void initializeTextBuffer(uint32_t fontType)
{
    m_scrollableBuffer = m_textBufferManager->createTextBuffer(fontType, BufferType::Transient);
	m_textBuffer = m_textBufferManager->createTextBuffer(fontType, BufferType::Transient);
}

void applyTextBufferAttributes()
{
    m_textBufferManager->setTextColor(m_scrollableBuffer, 0xFFFFFFFF);
	m_textBufferManager->setTextColor(m_textBuffer, 0xFFFFFFFF);
}

emscripten::val allocateMatrix() {
    emscripten::val result = emscripten::val::object();
    result.set("view_matrix", reinterpret_cast<uintptr_t>(view_matrix));
    result.set("projection_matrix", reinterpret_cast<uintptr_t>(projection_matrix));
    result.set("textProjection_matrix", reinterpret_cast<uintptr_t>(textProjection_matrix));
    return result;
}

void init()
{

    bgfx::PlatformData platformData{};

    platformData.nwh = (void*)"#canvas";

    platformData.context = NULL;
    platformData.backBuffer = NULL;
    platformData.backBufferDS = NULL;

    bgfx::Init init;
    init.type = bgfx::RendererType::OpenGLES;

    init.resolution.width = screenWidth;
    init.resolution.height = screenHeight;
    init.resolution.reset = BGFX_RESET_VSYNC;
    init.platformData = platformData;

    if (!bgfx::init(init))
    {
        throw std::runtime_error("Failed to initialize bgfx");
    }

    initializeFont(FONT_TYPE_DISTANCE);

    uint32_t size;
    uint32_t size1;

    m_bigText = bx::StringView((const char*)"test", size);
    bx::StringView test = bx::StringView((const char*)"bgfx", size1);

    m_lineCount = m_metrics.getLineCount(m_bigText);

    float m_visibleLineCount = 20.0f;

    m_textBegin = 0;
    m_textEnd = 0;
    m_metrics.getSubText(m_bigText, 0, (uint32_t)m_visibleLineCount, m_textBegin, m_textEnd);

    m_textBegin1 = 0;
    m_textEnd1 = 0;
    m_metrics.getSubText(test, 0, (uint32_t)m_visibleLineCount, m_textBegin1, m_textEnd1);

    initializeTextBuffer(FONT_TYPE_DISTANCE);

    applyTextBufferAttributes();

    m_textBufferManager->appendText(m_textBuffer, m_fontScaled, m_textBegin1, m_textEnd1);
    m_textBufferManager->appendText(m_scrollableBuffer, m_fontScaled, m_textBegin, m_textEnd);

    bgfx::setViewClear(0, BGFX_CLEAR_NONE, 0x443355FF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, screenWidth, screenHeight);

    bgfx::setViewClear(1, BGFX_CLEAR_NONE, 0x443355FF, 1.0f, 0);
    bgfx::setViewRect(1, 0, 0, screenWidth, screenHeight);

    emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");
	attr.onsuccess = downloadSucceed;
	attr.onerror = downloadFailed;
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    emscripten_fetch(&attr, "https://s3-us-east-2.amazonaws.com/ebiz-staging/uploads/archive_photo/photo/9409/b7b12801ce8ee8cd738b6d9161f8a687.png");

    PlaneGeometry::vbh = bgfx::createVertexBuffer(bgfx::makeRef(PlaneGeometry::PosTexCoord0Vertex, sizeof(PlaneGeometry::PosTexCoord0Vertex)), PlaneGeometry::vertexLayout);
	PlaneGeometry::ibh = bgfx::createIndexBuffer(bgfx::makeRef(PlaneGeometry::indices, sizeof(PlaneGeometry::indices)));
    MeshBasicMaterial::program = m_textBufferManager->m_mesh_program;
    MeshBasicMaterial::u_texture = bgfx::createUniform(
        "u_texture",
        bgfx::UniformType::Sampler
    );

    view_matrix = static_cast<float*>(malloc(16*sizeof(float)));
    projection_matrix = static_cast<float*>(malloc(16*sizeof(float)));
    textProjection_matrix = static_cast<float*>(malloc(16*sizeof(float)));

    // 注意：渲染循环现在由JavaScript控制，不再使用emscripten_set_main_loop
    // emscripten_set_main_loop(renderFrame, 0, 0);
}

EMSCRIPTEN_BINDINGS(GSBD) {
	emscripten::function("addNodeByScreenXY", &addNodeByScreenXY);
    emscripten::function("onWindowResize", &onWindowResize);
	emscripten::function("intersectNode", &intersectNode);
	emscripten::function("renderFrame", &renderFrame);
    emscripten::function("allocateMatrix", &allocateMatrix);
    emscripten::function("init", &init);
}
