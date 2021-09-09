#pragma once
#include <cmath>
#include <assert.h>

// Surface is just a buffer of Colors (dword-style)
class Color
{
public:
    unsigned int dword;
public:
    constexpr Color() noexcept
        :
        dword()
    {}
    constexpr Color(const Color& col) noexcept
        :
        dword(col.dword)
    {}
    constexpr Color(unsigned int dw) noexcept
        :
        dword(dw)
    {}
    constexpr Color(unsigned char r, unsigned char g, unsigned char b) noexcept
        :
        dword((r << 16u) | (g << 8u) | b)
    {}
    constexpr Color(unsigned char x, unsigned char r, unsigned char g, unsigned char b) noexcept
        :
        dword((x << 24u) | (r << 16u) | (g << 8u) | b)
    {}
    constexpr Color(Color col, unsigned char x) noexcept
        :
        Color((x << 24u) | col.dword)
    {}
    static void SetRandomSeed(unsigned int seed)
    {
        srand(seed);
    }
    static Color Random()
    {
        return Color((unsigned char)(rand() % 255u), (unsigned char)(rand() % 255u), (unsigned char)(rand() % 255u));
    }
    static constexpr Color FromRGB(float r, float g, float b)
    {
        return Color((unsigned char)(255.0f * r), (unsigned char)(255.0f * g), (unsigned char)(255.0f * b));
    }
    static Color FromHSV(float hueRad, float saturation = 1.0f, float value = 1.0f)
    {
        static constexpr float PI = 3.14159265358979f;
        static constexpr float twoPI = 2.0f * PI;

        while (hueRad < 0.0f)
        {
            hueRad += twoPI;
        }
        float hue = std::fmodf(hueRad * (180.0f / PI), 360.0f);

        assert(hue >= 0.0f);
        assert(hue < 360.0f);

        const float c = value * saturation;
        const float magic = 1.0f - std::abs(std::fmodf(hue / 60.0f, 2.0f) - 1.0f);
        const float x = c * magic;
        const float m = value - c;

        struct Float3
        {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
        };
        Float3 out;

        if (0.0f <= hue && hue < 60.0f)
        {
            out = { c,x,0.0f };
        }
        else if (60.0f <= hue && hue < 120.0f)
        {
            out = { x,c,0.0f };
        }
        else if (120.0f <= hue && hue < 180.0f)
        {
            out = { 0.0f,c,x };
        }
        else if (180.0f <= hue && hue < 240.0f)
        {
            out = { 0.0f,x,c };
        }
        else if (240.0f <= hue && hue < 300.0f)
        {
            out = { x,0.0f,c };
        }
        else if (300.0f <= hue && hue <= 360.0f)
        {
            out = { c,0.0f,x };
        }

        return Color((unsigned char)(255.0f * (out.x + m)), (unsigned char)(255.0f * (out.y + m)), (unsigned char)(255.0f * (out.z + m)));
    }

public:
    Color& operator = (Color color) noexcept
    {
        dword = color.dword;
        return *this;
    }
    constexpr unsigned char GetX() const noexcept
    {
        return dword >> 24u;
    }
    constexpr unsigned char GetR() const noexcept
    {
        return (dword >> 16u) & 0xFFu;
    }
    constexpr unsigned char GetG() const noexcept
    {
        return (dword >> 8u) & 0xFFu;
    }
    constexpr unsigned char GetB() const noexcept
    {
        return dword & 0xFFu;
    }
    void SetX(unsigned char x) noexcept
    {
        dword = (dword & 0xFFFFFFu) | (x << 24u);
    }
    void SetA(unsigned char a) noexcept
    {
        SetX(a);
    }
    void SetR(unsigned char r) noexcept
    {
        dword = (dword & 0xFF00FFFFu) | (r << 16u);
    }
    void SetG(unsigned char g) noexcept
    {
        dword = (dword & 0xFFFF00FFu) | (g << 8u);
    }
    void SetB(unsigned char b) noexcept
    {
        dword = (dword & 0xFFFFFF00u) | b;
    }
public:
    enum
    {
        Black                = 0xFF000000, // (0, 0, 0)
        White                = 0xFFFFFFFF, // (255, 255, 255)
        Red                  = 0xFFFF0000, // (255, 0, 0)
        Green                = 0xFF00FF00, // (0, 255, 0)
        Blue                 = 0xFF0000FF, // (0, 0, 255)
        Yellow               = 0xFFFFFF00, // (255, 255, 0)
        Aqua                 = 0xFF00FFFF, // (0, 255, 255)
        Magenta              = 0xFFFF00FF, // (255, 0, 255)
        Silver               = 0xFFC0C0C0, // (192, 192, 192)
        Gray                 = 0xFF808080, // (128, 128, 128)
        Maroon               = 0xFF800000, // (128, 0, 0)
        Olive                = 0xFF808000, // (128, 128, 0)
        Purple               = 0xFF800080, // (128, 0, 128)
        Teal                 = 0xFF008080, // (0, 128, 128)
        Navy                 = 0xFF000080, // (0, 0, 128)
        DarkRed	             = 0xFF8B0000, // (139, 0, 0)
        Brown	             = 0xFFA52A2A, // (165, 42, 42)
        Firebrick	         = 0xFFB22222, // (178, 34, 34)
        Crimson	             = 0xFFDC143C, // (220, 20, 60)
        Tomato	             = 0xFFFF6347, // (255, 99, 71)
        Coral	             = 0xFFFF7F50, // (255, 127, 80)
        IndianRed	         = 0xFFCD5C5C, // (205, 92, 92)
        LightCoral	         = 0xFFF08080, // (240, 128, 128)
        DarkSalmon	         = 0xFFE9967A, // (233, 150, 122)
        Salmon	             = 0xFFFA8072, // (250, 128, 114)
        LightSalmon          = 0xFFFFA07A, // (255, 160, 122)
        OrangeRed	         = 0xFFFF4500, // (255, 69, 0)
        DarkOrange	         = 0xFFFF8C00, // (255, 140, 0)
        Orange	             = 0xFFFFA500, // (255, 165, 0)
        Gold	             = 0xFFFFD700, // (255, 215, 0)
        DarkGoldenRrod	     = 0xFFB8860B, // (184, 134, 11)
        GoldenRod	         = 0xFFDAA520, // (218, 165, 32)
        PaleGoldenRod	     = 0xFFEEE8AA, // (238, 232, 170)
        DarkKhaki	         = 0xFFBDB76B, // (189, 183, 107)
        Khaki	             = 0xFFF0E68C, // (240, 230, 140)
        YellowGreen	         = 0xFF9ACD32, // (154, 205, 50)
        DarkOliveGreen	     = 0xFF556B2F, // (85, 107, 47)
        OliveDrab	         = 0xFF6B8E23, // (107, 142, 35)
        LawnGreen	         = 0xFF7CFC00, // (124, 252, 0)
        ChartReuse	         = 0xFF7FFF00, // (127, 255, 0)
        GreenYellow	         = 0xFFADFF2F, // (173, 255, 47)
        DarkGreen	         = 0xFF006400, // (0, 100, 0)
        ForestGreen	         = 0xFF228B22, // (34, 139, 34)
        Lime	             = 0xFF00FF00, // (0, 255, 0)
        LimeGreen	         = 0xFF32CD32, // (50, 205, 50)
        LightGreen	         = 0xFF90EE90, // (144, 238, 144)
        PaleGreen	         = 0xFF98FB98, // (152, 251, 152)
        DarkSeaGreen	     = 0xFF8FBC8F, // (143, 188, 143)
        MediumSpringGreen	 = 0xFF00FA9A, // (0, 250, 154)
        SpringGreen	         = 0xFF00FF7F, // (0, 255, 127)
        SeaGreen	         = 0xFF2E8B57, // (46, 139, 87)
        MediumAquaMarine	 = 0xFF66CDAA, // (102, 205, 170)
        MediumSeaGreen	     = 0xFF3CB371, // (60, 179, 113)
        LightSeaGreen	     = 0xFF20B2AA, // (32, 178, 170)
        DarkSlateGray	     = 0xFF2F4F4F, // (47, 79, 79)
        DarkCyan	         = 0xFF008B8B, // (0, 139, 139)
        Cyan	             = 0xFF00FFFF, // (0, 255, 255)
        LightCyan	         = 0xFFE0FFFF, // (224, 255, 255)
        DarkTurquoise	     = 0xFF00CED1, // (0, 206, 209)
        Turquoise	         = 0xFF40E0D0, // (64, 224, 208)
        MediumTurquoise	     = 0xFF48D1CC, // (72, 209, 204)
        PaleTurquoise	     = 0xFFAFEEEE, // (175, 238, 238)
        AquaMarine	         = 0xFF7FFFD4, // (127, 255, 212)
        PowderBlue	         = 0xFFB0E0E6, // (176, 224, 230)
        CadetBlue	         = 0xFF5F9EA0, // (95, 158, 160)
        SteelBlue	         = 0xFF4682B4, // (70, 130, 180)
        CornFlowerBlue	     = 0xFF6495ED, // (100, 149, 237)
        DeepSkyBlue	         = 0xFF00BFFF, // (0, 191, 255)
        DodgerBlue	         = 0xFF1E90FF, // (30, 144, 255)
        LightBlue	         = 0xFFADD8E6, // (173, 216, 230)
        SkyBlue	             = 0xFF87CEEB, // (135, 206, 235)
        LightSkyBlue	     = 0xFF87CEFA, // (135, 206, 250)
        MidnightBlue	     = 0xFF191970, // (25, 25, 112)
        DarkBlue	         = 0xFF00008B, // (0, 0, 139)
        MediumBlue	         = 0xFF0000CD, // (0, 0, 205)
        RoyalBlue	         = 0xFF4169E1, // (65, 105, 225)
        BlueViolet	         = 0xFF8A2BE2, // (138, 43, 226)
        Indigo	             = 0xFF4B0082, // (75, 0, 130)
        DarkSlateBlue	     = 0xFF483D8B, // (72, 61, 139)
        SlateBlue	         = 0xFF6A5ACD, // (106, 90, 205)
        MediumSlateBlue	     = 0xFF7B68EE, // (123, 104, 238)
        MediumPurple	     = 0xFF9370DB, // (147, 112, 219)
        DarkMagenta	         = 0xFF8B008B, // (139, 0, 139)
        DarkViolet	         = 0xFF9400D3, // (148, 0, 211)
        DarkOrchid	         = 0xFF9932CC, // (153, 50, 204)
        MediumOrchid	     = 0xFFBA55D3, // (186, 85, 211)
        Thistle	             = 0xFFD8BFD8, // (216, 191, 216)
        Plum	             = 0xFFDDA0DD, // (221, 160, 221)
        Violet	             = 0xFFEE82EE, // (238, 130, 238)
        Orchid	             = 0xFFDA70D6, // (218, 112, 214)
        MediumVioletRed	     = 0xFFC71585, // (199, 21, 133)
        PaleVioletRed	     = 0xFFDB7093, // (219, 112, 147)
        DeepPink	         = 0xFFFF1493, // (255, 20, 147)
        HotPink	             = 0xFFFF69B4, // (255, 105, 180)
        LightPink	         = 0xFFFFB6C1, // (255, 182, 193)
        Pink	             = 0xFFFFC0CB, // (255, 192, 203)
        AntiqueWhite	     = 0xFFFAEBD7, // (250, 235, 215)
        Beige	             = 0xFFF5F5DC, // (245, 245, 220)
        Bisque	             = 0xFFFFE4C4, // (255, 228, 196)
        BlanchedAlmond	     = 0xFFFFEBCD, // (255, 235, 205)
        Wheat	             = 0xFFF5DEB3, // (245, 222, 179)
        CornSilk	         = 0xFFFFF8DC, // (255, 248, 220)
        LemonChiffon	     = 0xFFFFFACD, // (255, 250, 205)
        LightGoldenRodYellow = 0xFFFAFAD2, // (250, 250, 210)
        LightYellow	         = 0xFFFFFFE0, // (255, 255, 224)
        SaddleBrown	         = 0xFF8B4513, // (139, 69, 19)
        Sienna	             = 0xFFA0522D, // (160, 82, 45)
        Chocolate	         = 0xFFD2691E, // (210, 105, 30)
        Peru	             = 0xFFCD853F, // (205, 133, 63)
        SandyBrown	         = 0xFFF4A460, // (244, 164, 96)
        BurlyWood	         = 0xFFDEB887, // (222, 184, 135)
        Tan	                 = 0xFFD2B48C, // (210, 180, 140)
        RosyBrown	         = 0xFFBC8F8F, // (188, 143, 143)
        Moccasin	         = 0xFFFFE4B5, // (255, 228, 181)
        NavajoWhite	         = 0xFFFFDEAD, // (255, 222, 173)
        PeachPuff	         = 0xFFFFDAB9, // (255, 218, 185)
        MistyRose	         = 0xFFFFE4E1, // (255, 228, 225)
        LavenderBlush	     = 0xFFFFF0F5, // (255, 240, 245)
        Linen	             = 0xFFFAF0E6, // (250, 240, 230)
        OldLace	             = 0xFFFDF5E6, // (253, 245, 230)
        PapayaWhip	         = 0xFFFFEFD5, // (255, 239, 213)
        SeaShell	         = 0xFFFFF5EE, // (255, 245, 238)
        MintCream	         = 0xFFF5FFFA, // (245, 255, 250)
        SlateGray	         = 0xFF708090, // (112, 128, 144)
        LightSlateGray	     = 0xFF778899, // (119, 136, 153)
        LightSteelBlue	     = 0xFFB0C4DE, // (176, 196, 222)
        Lavender	         = 0xFFE6E6FA, // (230, 230, 250)
        FloralWhite	         = 0xFFFFFAF0, // (255, 250, 240)
        AliceBlue	         = 0xFFF0F8FF, // (240, 248, 255)
        GhostWhite	         = 0xFFF8F8FF, // (248, 248, 255)
        Honeydew	         = 0xFFF0FFF0, // (240, 255, 240)
        Ivory	             = 0xFFFFFFF0, // (255, 255, 240)
        Azure	             = 0xFFF0FFFF, // (240, 255, 255)
        Snow	             = 0xFFFFFAFA, // (255, 250, 250)
        DimGray        	     = 0xFF696969, // (105, 105, 105)
        DarkGray             = 0xFFA9A9A9, // (169, 169, 169)
        LightGray            = 0xFFD3D3D3, // (211, 211, 211)
        Gainsboro	         = 0xFFDCDCDC, // (220, 220, 220)
        WhiteSmoke	         = 0xFFF5F5F5, // (245, 245, 245)
    };
};