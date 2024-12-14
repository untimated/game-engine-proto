#include <cstdint>
#include <platform/FontLoader.h>
#include <EnginePlatformAPI.h>
#include <algorithm>
#include <utils/Debug.h>

using namespace FontLoader;
FT_Library ftlib;


bool FontLoader::Initialize() {
    FT_Error error;
    error = FT_Init_FreeType(&ftlib);
    if(IsError(error)) return false;

    // int spread = 2;
    // error = FT_Property_Set(ftlib, "sdf", "spread", &spread);
    // if(IsError(error)) return false;

    return true;
}


bool FontLoader::Shutdown() {
    FT_Error err = FT_Done_FreeType(ftlib);
    if(IsError(err)) return false;
    return true;
}


FontLoader::Font* FontLoader::LoadFont(const char* path, uint32_t size) {

    FT_Error error;
    FT_Face face;
    error = FT_New_Face(ftlib, path, 0, &face);
    if(IsError(error)) return nullptr;

    uint32_t dpi = EnginePlatformAPI::GetScreenDPI();
    error = FT_Set_Char_Size(face, 0, size * 64, dpi, dpi );
    if(face->available_sizes) {
        for(int i = 0; i < face->num_fixed_sizes; i++) {
            Debug::Logger("FontLoader::", "available size :", face->available_sizes[i].size >> 6);
        }
    }else{
        Debug::Logger("FontLoader::", "no available sizes");
    }
    if(IsError(error)) return nullptr;

    Debug::Logger("FontLoader::", "target font size in pt", size);
    Debug::Logger("FontLoader::", "screen dpi", dpi);
    Debug::Logger("FontLoader::", "face.family_name", face->family_name);
    Debug::Logger("FontLoader::", "face.style_name", face->style_name);
    Debug::Logger("FontLoader::", "face.num_glyphs", face->num_glyphs);
    Debug::Logger("FontLoader::", "face.face_flags", face->face_flags);
    Debug::Logger("FontLoader::", "face.metrics.ascender", face->size->metrics.ascender >> 6);
    Debug::Logger("FontLoader::", "face.metrics.descender", face->size->metrics.descender >> 6);
    Debug::Logger("FontLoader::", "face.metrics.ascender + descender", (face->size->metrics.ascender + std::abs(face->size->metrics.descender)) >> 6);
    Debug::Logger("FontLoader::", "face.metrics.height", face->size->metrics.height >> 6);

    Font *newFont = new Font();
    FontAtlas *atlas = new FontAtlas();
    Debug::Logger("Size of FontAtlas", sizeof(FontAtlas));

    for(uint8_t code = 0; code <= 127; code++) {
        error = FT_Load_Glyph(face, FT_Get_Char_Index(face, code), FT_LOAD_TARGET_NORMAL);
        // error = FT_Load_Glyph(face, FT_Get_Char_Index(face, code), FT_LOAD_NO_HINTING);
        // error = FT_Load_Glyph(face, FT_Get_Char_Index(face, code), FT_LOAD_TARGET_LIGHT | FT_LOAD_TARGET_LCD);
        if(IsError(error)) return nullptr;

        // error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);
        if(IsError(error)) return nullptr;

        uint8_t* glyphBuffer = face->glyph->bitmap.buffer;
        uint32_t bw = face->glyph->bitmap.width;
        uint32_t bh = face->glyph->bitmap.rows;
        uint32_t w = face->glyph->metrics.width >> 6;
        uint32_t h = face->glyph->metrics.height >> 6;
        uint32_t dim = bw * bh;
        uint32_t outBufferSize = sizeof(RGBA) * dim;
        RGBA *outBuffer = new RGBA[outBufferSize];
        SaveToBuffer(glyphBuffer, outBuffer, bw, bh);

        Glyph* newGlyph = new Glyph();
        newGlyph->buffer = outBuffer;
        newGlyph->bufferSize = outBufferSize;
        newGlyph->width = bw;
        newGlyph->height = bh;
        newGlyph->advance = face->glyph->advance.x >> 6; // shift right 6 is equivalent to division by 2^6 (64)
        newGlyph->character = code;
        newGlyph->bearingX = face->glyph->bitmap_left;
        newGlyph->bearingY = face->glyph->bitmap_top;
        
        atlas->glyphs[code] = newGlyph;
    }

    atlas->height       = face->size->metrics.height >> 6;
    atlas->ascender     = face->size->metrics.ascender >> 6;
    atlas->descender    = face->size->metrics.descender >> 6;
    newFont->family     = std::string(face->family_name);

    newFont->atlas = atlas;
    newFont->size = size;
    Debug::Logger("FontLoader::", "atlas created", face->family_name);

    return newFont;
}


bool FontLoader::RenderText(RGBA** surfaceBuffer, Font* font, const char* text, uint32_t &width, uint32_t &height) {
    FontLoader::FontAtlas *atlas = font->atlas;

    uint32_t currentLineWidth = 0;
    uint32_t textTotalWidth = 0;
    uint32_t textMaxHeight  = atlas->height;
    uint32_t textLength     = strlen(text);
    FontLoader::Glyph** textGlyphs = new FontLoader::Glyph*[textLength];
    for(int i = 0; i < textLength; i++) {
        char code = text[i];
        if(code > 127) {
            Debug::Logger("FontLoader:: Character out of bound or not supported by text engine -", code);
        }
        Glyph *current = atlas->glyphs[code];
        textGlyphs[i] = current;
        if(code == '\n') {
            textMaxHeight += atlas->height;
            textTotalWidth = std::max<int>(textTotalWidth, currentLineWidth);
            currentLineWidth = 0;
        }else{
            currentLineWidth += current->advance;
        }
    }
    textTotalWidth = std::max<int>(textTotalWidth, currentLineWidth);
    textTotalWidth += 1; // extra gutter
    width = textTotalWidth;
    height = textMaxHeight;

    *surfaceBuffer = new RGBA[textMaxHeight * textTotalWidth];
    RGBA* surface = *surfaceBuffer;
    size_t pitch = sizeof(RGBA);
    int penX = 0; int penY = 0;
    int currentLine = 0;
    for(int i = 0; i < textLength; i++) {
        Glyph* current = textGlyphs[i];
        if(current->character == '\n') {
            currentLine +=atlas->height;
            penX = 0;
            continue;
        }

        int leading = atlas->ascender - current->bearingY;
        penY        = (std::max<int>(leading, 0) + currentLine) * textTotalWidth;
        int surfaceX = 0; int glyphX = 0;
        int surfaceY = 0; int glyphY = 0;
        surfaceX = current->bearingX;
        while(glyphX < current->width){
            if(surfaceX < 0) {
                surfaceX++; glyphX++;
                continue;
            }
            if(penX + surfaceX >= textTotalWidth) { break;}
            surfaceY = 0;
            glyphY = leading < 0 ? std::abs(leading) : 0;
            while(glyphY < current->height) {
                if(std::max<int>(leading, 0) + currentLine + surfaceY >= textMaxHeight) { break; }
                memcpy(
                    &surface[(penY + (surfaceY * textTotalWidth)) + (penX + surfaceX)],
                    &current->buffer[(glyphY * current->width) + glyphX],
                    pitch
                    );
                surfaceY++; glyphY++;
            }
            surfaceX++; glyphX++;
        }
        penX += current->advance;
    } 

    // PrintGlyphBuffer(surface, textTotalWidth, textMaxHeight);
    // Debug::Logger("final buffer size of our rendered text", (sizeof(*surface) * textTotalWidth * textMaxHeight));

    delete[] textGlyphs;

    return true;

}


bool FontLoader::RenderTextBox(RGBA** surfaceBuffer, Font* font, const char* text, uint32_t boxW, uint32_t boxH) {
    FontLoader::FontAtlas *atlas = font->atlas;

    uint32_t currentLineWidth = 0;
    uint32_t textTotalWidth = boxW;
    uint32_t textMaxHeight  = boxH;
    uint32_t textLength     = strlen(text);
    FontLoader::Glyph** textGlyphs = new FontLoader::Glyph*[textLength];
    for(int i = 0; i < textLength; i++) {
        char code = text[i];
        if(code > 127) {
            Debug::Logger("FontLoader:: Character out of bound or not supported by text engine -", code);
        }
        Glyph *current = atlas->glyphs[code];
        textGlyphs[i] = current;
    }

    *surfaceBuffer = new RGBA[textMaxHeight * textTotalWidth];
    RGBA* surface = *surfaceBuffer;
    size_t pitch = sizeof(RGBA);
    int penX = 0; int penY = 0;
    int currentLine = 0;
    for(int i = 0; i < textLength; i++) {
        Glyph* current = textGlyphs[i];
        if(current->character == '\n') {
            currentLine +=atlas->height;
            penX = 0;
            continue;
        }

        int leading = atlas->ascender - current->bearingY;
        penY        = (std::max<int>(leading, 0) + currentLine) * textTotalWidth;
        int surfaceX = 0; int glyphX = 0;
        int surfaceY = 0; int glyphY = 0;
        surfaceX = current->bearingX;
        while(glyphX < current->width){
            if(surfaceX < 0) {
                surfaceX++; glyphX++;
                continue;
            }
            if(penX + surfaceX >= textTotalWidth) { break;}
            surfaceY = 0;
            glyphY = leading < 0 ? std::abs(leading) : 0;
            while(glyphY < current->height) {
                if(std::max<int>(leading, 0) + currentLine + surfaceY >= textMaxHeight) { break; }
                memcpy(
                    &surface[(penY + (surfaceY * textTotalWidth)) + (penX + surfaceX)],
                    &current->buffer[(glyphY * current->width) + glyphX],
                    pitch
                    );
                surfaceY++; glyphY++;
            }
            surfaceX++; glyphX++;
        }
        penX += current->advance;
    } 

    // PrintGlyphBuffer(surface, textTotalWidth, textMaxHeight);
    // Debug::Logger("final buffer size of our rendered text", (sizeof(*surface) * textTotalWidth * textMaxHeight));

    delete[] textGlyphs;

    return true;

}


bool FontLoader::FreeFont(Font *font) {
    FontAtlas* atlas = font->atlas;
    for(int i = 0; i < 255; i++) {
        if(atlas->glyphs[i] == nullptr) continue;
        delete atlas->glyphs[i]->buffer;
        delete atlas->glyphs[i];
        atlas->glyphs[i] = nullptr;
    }
    delete atlas;
    Debug::Logger("Font cleared", font->family);
    return true;
}


static bool FontLoader::IsError(FT_Error error) {
    if(error != FT_Err_Ok) {
        const char* msg = FT_Error_String(error);
        Debug::Logger("Error FreeType", msg);
        return true;
    }
    return false;
}


void FontLoader::SaveToBuffer(unsigned char* inBuffer, RGBA* outBuffer, uint32_t w, uint32_t h) {
    RGBA* tmp = outBuffer;
    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j += 1) {
            uint8_t a = inBuffer[(i*w) + j];
            uint8_t r = a > 0 ? 255 : 0;
            outBuffer->r = r;
            outBuffer->g = r;
            outBuffer->b = r;
            outBuffer->a = a;
            outBuffer++;
        }
    }
    outBuffer = tmp;
}


void FontLoader::PrintGlyphBuffer(RGBA* buffer, uint32_t w, uint32_t h) {
    RGBA* tmp = buffer;
    for(int i = 0; i < h; i++) {
        std::string row;
        for(int j = 0; j < w; j++) {
            uint8_t _r = std::clamp<uint8_t>(tmp->r, 0, 1);
            row += (_r ? "*" : "-");
            row += " ";
            tmp++;
        }
        Debug::Logger(row);
    }
}


static void FontLoader::PrintGlyphDetail(FT_GlyphSlot glyph) {
    Debug::Logger("Glyph width", glyph->bitmap.width);
    Debug::Logger("Glyph height", glyph->bitmap.rows);
    Debug::Logger("Glyph metrics width", glyph->metrics.width >> 6);
    Debug::Logger("Glyph metrics height", glyph->metrics.height >> 6);
    Debug::Logger("Glyph advance value x/64", glyph->advance.x >> 6);
    Debug::Logger("Glyph hori advance ", glyph->linearHoriAdvance >> 16);
    Debug::Logger("Glyph bitmap left (bearingX)", glyph->bitmap_left);
    Debug::Logger("Glyph bitmap top (bearingY)", glyph->bitmap_top);
    Debug::Logger("Glyph bitmap pitch", glyph->bitmap.pitch);
}


/* void FontLoader::SaveAsPixMap(unsigned char* buffer, uint32_t w, uint32_t h) {
    std::fstream fs("./glyph.pbm", std::fstream::out | std::fstream::binary);

    std::string header;
    // header += "P3\n";
    header += "P6\n";
    header += "#MyFontPixmap\n" ;
    header += std::to_string(w) + " " + std::to_string(h) + "\n";
    header += "255\n";
    fs.write(header.c_str(), header.size());

    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j++) {
            uint32_t value = (uint32_t) buffer[(i*w) + j];
            fs.put(value);
            fs.put(value);
            fs.put(value);
        }
    }
    fs.close();
} */


