#ifndef FONT_LOADER_H
#define FONT_LOADER_H

#include <cstdint>
#include <unordered_map>
#include <ft2build.h>
#include <string>
#include <freetype/ftglyph.h>
#include <freetype/ftmodapi.h>
#include FT_FREETYPE_H

struct RGBA {
    uint8_t r=0;
    uint8_t g=0;
    uint8_t b=0;
    uint8_t a=0;
};


namespace FontLoader {

    struct Glyph {
        RGBA *buffer;
        uint32_t bufferSize;
        uint32_t width;
        uint32_t height;
        uint32_t advance;
        int32_t bearingX;
        int32_t bearingY;
        char character;
    };

    struct FontAtlas {
        Glyph* glyphs[255] = {nullptr};
        uint32_t ascender;
        uint32_t descender;
        uint32_t height;
    };

    struct Font {
        std::string family;
        uint32_t size;
        FontAtlas *atlas;
        // std::unordered_map<uint32_t, FontAtlas*> atlas;
    };

    bool Initialize();
    bool Shutdown();

    Font* LoadFont(const char* path, uint32_t size);
    bool RenderText(RGBA** surfaceBuffer, Font* font, const char* text, uint32_t &width, uint32_t &height);
    bool RenderTextBox(RGBA** surfaceBuffer, Font* font, const char* text, uint32_t boxW, uint32_t boxH);
    bool FreeFont(Font *font);

    static bool IsError(FT_Error);
    void DrawGlyph(Glyph glyph, uint8_t* glyphBuffer, uint32_t targetWidth, uint32_t targetHeight);
    void SaveToBuffer(unsigned char* inBuffer, RGBA* outBuffer, uint32_t w, uint32_t h);

    void PrintGlyphBuffer(RGBA* buffer, uint32_t w, uint32_t h);
    static void PrintGlyphDetail(FT_GlyphSlot glyph);
    // void SaveAsPixMap(unsigned char* buffer, uint32_t w, uint32_t h);
}

#endif
