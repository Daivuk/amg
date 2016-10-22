#include <atlbase.h>
#include <msxml6.h>
#include <comutil.h>
#include <atlimage.h>

#include <future>
#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "asciipng.h"

using namespace std;

std::string setting_url = "http://blog-photocontact.vellozzi.com/wp-content/uploads/2011/10/Martin-Schoeller-Barack-Obama-portrait.jpeg";
bool        setting_gamma = true;
bool        setting_filter = true;
bool        setting_multi = true;
bool        setting_influence = true;
bool        setting_simple = false;
bool        setting_random = false;
int         setting_width = 79;

map<float, vector<char>> bToC =
{
    {0.f, {' ', }},
    {0.133333f, {'.', /*'`',*/ }},
    {0.155556f, {'-', }},
    {0.177778f, {/*'\'', */','/*, '_',*/ }},
    {0.266667f, {':', /*'=', '^', */}},
    {0.311111f, {/*'"', */'+',/* '/', '\\',*/ }},
    {0.333333f, {'~', }},
    {0.355556f, {';',/* '|', */}},
    {0.4f, {'(', /*')', '<', '>', */}},
    {0.444444f, {'%'/*, '?', 'c', 's', '{', '}',*/ }},
    {0.488889f, {/*'!', 'I', '[', ']', 'i', 't', 'v',*/ 'x',/* 'z',*/ }},
    {0.511111f, {'1'/*, 'r', */}},
    {0.533333f, {'*'/*, 'a', 'e', 'l', 'o', */}},
    {0.555556f, {'n', 'u', }},
    {0.577778f, {'T', /*'f', 'w', */}},
    {0.6f, {'3',/* '7',*/ }},
    {0.622222f, {'J', /*'j', 'y',*/ }},
    {0.644444f, {'5', }},
    {0.666667f, {'$',/* '2', '6', '9', 'C', 'L', 'Y', 'm',*/ }},
    {0.688889f, {'S', }},
    {0.711111f, {'4',/* 'g', 'k', 'p', 'q', */}},
    {0.733333f, {'F', 'P',/* 'b', 'd', 'h',*/ }},
    {0.755556f, {'G', 'O', 'V', 'X', }},
    {0.777778f, {'E', 'Z', }},
    {0.8f, {'8', 'A', 'U', }},
    {0.844444f, {'D', 'H', 'K', 'W', }},
    {0.888889f, {/*'&',*/ '@',/* 'R', */}},
    {0.911111f, {'B', 'Q', }},
    {0.933333f, {'#', }},
    {1.f, {'0', 'M', 'N', }},
};

map<float, vector<char>> bToC_simple =
{
    {0.f, {' '}},
    {0.25f, {'-'}},
    {0.5f, {'+'}},
    {0.75f, {'*'}},
    {1.f, {'#'}},
};

map<float, vector<char>> bToC_full =
{
    {0.f, {' ', }},
    {0.133333f, {'.', '`', }},
    {0.155556f, {'-', }},
    {0.177778f, {'\'', ',', '_', }},
    {0.266667f, {':', '=', '^', }},
    {0.311111f, {'"', '+', '/', '\\', }},
    {0.333333f, {'~', }},
    {0.355556f, {';', '|', }},
    {0.4f, {'(', ')', '<', '>', }},
    {0.444444f, {'%', '?', 'c', 's', '{', '}', }},
    {0.488889f, {'!', 'I', '[', ']', 'i', 't', 'v', 'x', 'z', }},
    {0.511111f, {'1', 'r', }},
    {0.533333f, {'*', 'a', 'e', 'l', 'o', }},
    {0.555556f, {'n', 'u', }},
    {0.577778f, {'T', 'f', 'w', }},
    {0.6f, {'3', '7', }},
    {0.622222f, {'J', 'j', 'y', }},
    {0.644444f, {'5', }},
    {0.666667f, {'$', '2', '6', '9', 'C', 'L', 'Y', 'm', }},
    {0.688889f, {'S', }},
    {0.711111f, {'4', 'g', 'k', 'p', 'q', }},
    {0.733333f, {'F', 'P', 'b', 'd', 'h', }},
    {0.755556f, {'G', 'O', 'V', 'X', }},
    {0.777778f, {'E', 'Z', }},
    {0.8f, {'8', 'A', 'U', }},
    {0.844444f, {'D', 'H', 'K', 'W', }},
    {0.888889f, {'&', '@', 'R', }},
    {0.911111f, {'B', 'Q', }},
    {0.933333f, {'#', }},
    {1.f, {'0', 'M', 'N', }},
};

unsigned long getPixelAVG(const CImage& image, int x, int y)
{
    auto pixel = image.GetPixel(x, y);
    auto a = (pixel >> 24) & 0xff;
    auto b = (pixel >> 16) & 0xff;
    auto g = (pixel >> 8) & 0xff;
    auto r = pixel & 0xff;

    return (r * 2126 + g * 7152 + b * 722) / 10000;
}

unsigned long getPixelAVGFromASCII(int x, int y)
{
    auto pixel = asciipngdata[y * 760 + x];
    auto a = (pixel >> 24) & 0xff;
    auto b = (pixel >> 16) & 0xff;
    auto g = (pixel >> 8) & 0xff;
    auto r = pixel & 0xff;

    return (r * 2126 + g * 7152 + b * 722) / 10000;
}

struct Char
{
    char c;
    union
    {
        float b[9];
        struct
        {
            float _0, _1, _2, _3, _4, _5, _6, _7, _8;
        };
    };
};

vector<Char> chars;

float getScore(const Char& ch, float* b)
{
    float score = 0.f;

    if (setting_influence)
    {
        score += abs(b[0] - ch.b[0]) * (2.f * 3.f);
        score += abs(b[1] - ch.b[1]) * (4.f * 3.f);
        score += abs(b[2] - ch.b[2]) * (2.f * 3.f);
        score += abs(b[3] - ch.b[3]) * (2.f * 6.f);
        score += abs(b[4] - ch.b[4]) * (6.f * 8.f);
        score += abs(b[5] - ch.b[5]) * (2.f * 6.f);
        score += abs(b[6] - ch.b[6]) * (2.f * 3.f);
        score += abs(b[7] - ch.b[7]) * (4.f * 3.f);
        score += abs(b[8] - ch.b[8]) * (2.f * 3.f);
    }
    else
    {
        score += abs(b[0] - ch.b[0]) * (2.f * 3.f);
        score += abs(b[1] - ch.b[1]) * (4.f * 3.f);
        score += abs(b[2] - ch.b[2]) * (2.f * 3.f);
        score += abs(b[3] - ch.b[3]) * (2.f * 6.f);
        score += abs(b[4] - ch.b[4]) * (4.f * 6.f);
        score += abs(b[5] - ch.b[5]) * (2.f * 6.f);
        score += abs(b[6] - ch.b[6]) * (2.f * 3.f);
        score += abs(b[7] - ch.b[7]) * (4.f * 3.f);
        score += abs(b[8] - ch.b[8]) * (2.f * 3.f);
    }

    return score;
}

#include <fstream>

void generateData()
{
    map<float, vector<char>> countToC;
    float maxB[9] = {0};
    for (auto c = ' '; c <= '~'; ++c)
    {
        auto offset = c - ' ';
        offset *= 8;
        int count[9] = {0};

        vector<float> brightness;

        if (setting_influence)
        {
            // Top left
            for (auto y = 0; y < 3; ++y)
                for (auto x = offset; x < offset + 2; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[0];

            // Top
            for (auto y = 0; y < 3; ++y)
                for (auto x = offset + 2; x < offset + 6; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[1];

            // Top right
            for (auto y = 0; y < 3; ++y)
                for (auto x = offset + 6; x < offset + 8; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[2];

            // Left
            for (auto y = 3; y < 9; ++y)
                for (auto x = offset + 0; x < offset + 2; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[3];

            // Center
            for (auto y = 2; y < 10; ++y)
                for (auto x = offset + 1; x < offset + 7; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[4];

            // Right
            for (auto y = 3; y < 9; ++y)
                for (auto x = offset + 6; x < offset + 8; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[5];

            // Bottom left
            for (auto y = 9; y < 12; ++y)
                for (auto x = offset + 0; x < offset + 2; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[6];

            // Bottom
            for (auto y = 9; y < 12; ++y)
                for (auto x = offset + 2; x < offset + 6; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[7];

            // Bottom right
            for (auto y = 9; y < 12; ++y)
                for (auto x = offset + 6; x < offset + 8; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[8];

            brightness = {
                (float)count[0] / (2.f * 3.f),
                (float)count[1] / (4.f * 3.f),
                (float)count[2] / (2.f * 3.f),
                (float)count[3] / (2.f * 6.f),
                (float)count[4] / (6.f * 8.f),
                (float)count[5] / (2.f * 6.f),
                (float)count[6] / (2.f * 3.f),
                (float)count[7] / (4.f * 3.f),
                (float)count[8] / (2.f * 3.f),
            };
        }
        else
        {
            // Top left
            for (auto y = 0; y < 3; ++y)
                for (auto x = offset; x < offset + 2; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[0];

            // Top
            for (auto y = 0; y < 3; ++y)
                for (auto x = offset + 2; x < offset + 6; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[1];

            // Top right
            for (auto y = 0; y < 3; ++y)
                for (auto x = offset + 6; x < offset + 8; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[2];

            // Left
            for (auto y = 3; y < 9; ++y)
                for (auto x = offset + 0; x < offset + 2; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[3];

            // Center
            for (auto y = 3; y < 9; ++y)
                for (auto x = offset + 2; x < offset + 6; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[4];

            // Right
            for (auto y = 3; y < 9; ++y)
                for (auto x = offset + 6; x < offset + 8; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[5];

            // Bottom left
            for (auto y = 9; y < 12; ++y)
                for (auto x = offset + 0; x < offset + 2; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[6];

            // Bottom
            for (auto y = 9; y < 12; ++y)
                for (auto x = offset + 2; x < offset + 6; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[7];

            // Bottom right
            for (auto y = 9; y < 12; ++y)
                for (auto x = offset + 6; x < offset + 8; ++x)
                    if (getPixelAVGFromASCII(x, y) > 150) ++count[8];

            brightness = {
                (float)count[0] / (2.f * 3.f),
                (float)count[1] / (4.f * 3.f),
                (float)count[2] / (2.f * 3.f),
                (float)count[3] / (2.f * 6.f),
                (float)count[4] / (4.f * 6.f),
                (float)count[5] / (2.f * 6.f),
                (float)count[6] / (2.f * 3.f),
                (float)count[7] / (4.f * 3.f),
                (float)count[8] / (2.f * 3.f),
            };
        }

        for (int i = 0; i < 9; ++i)
        {
            maxB[i] = max<>(maxB[i], brightness[i]);
        }

        chars.push_back({c, brightness[0], brightness[1], brightness[2], brightness[3], brightness[4], brightness[5], brightness[6], brightness[7]});
    }

    for (auto &ch : chars)
    {
        for (int i = 0; i < 9; ++i)
        {
            ch.b[i] /= maxB[i];
        }
    }
}

void readSettings(int argc, const char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (_stricmp(argv[i] + 1, "g") == 0)
                setting_gamma = false;
            else if (_stricmp(argv[i] + 1, "f") == 0)
                setting_filter = false;
            else if (_stricmp(argv[i] + 1, "m") == 0)
                setting_multi = false;
            else if (_stricmp(argv[i] + 1, "i") == 0)
                setting_influence = false;
            else if (_stricmp(argv[i] + 1, "s") == 0)
                setting_simple = true;
            else if (_stricmp(argv[i] + 1, "r") == 0)
                setting_random = true;
            else if (_stricmp(argv[i] + 1, "w") == 0)
                setting_width = atoi(argv[++i]);
        }
        else
        {
            setting_url = argv[i];
        }
    }
}

int main(int argc, const char* argv[])
{
    CoInitialize(nullptr);
    srand((unsigned int)time(0));

    readSettings(argc, argv);

    generateData();

    HRESULT hr;
    CComPtr<IXMLHTTPRequest> request;

    hr = request.CoCreateInstance(CLSID_XMLHTTP60);
    hr = request->open(_bstr_t("GET"),
                       _bstr_t(setting_url.c_str()),
                       _variant_t(VARIANT_FALSE),
                       _variant_t(),
                       _variant_t());

    auto ret = async(launch::async, [&]
    {
        HRESULT hr;

        hr = request->send(_variant_t());
    });

    ret.wait();

    // get status - 200 if success
    long status;
    hr = request->get_status(&status);

    if (status == 200)
    {
        // load image data (if URL points to an image)
        VARIANT responseVariant;
        hr = request->get_responseStream(&responseVariant);
        IStream* stream = (IStream*)responseVariant.punkVal;
        CImage image;
        image.Load(stream);
        stream->Release();

        auto w = image.GetWidth();
        auto h = image.GetHeight();
        float ratio = (float)w / (float)setting_width;
        auto ratioh = ratio * (12.f / 8.f);
        int sw = (int)((float)w / ratio);
        int sh = (int)((float)h / ratioh);

        unsigned long bAvg = 0;
        float fAvg = 0.f;
        if (setting_gamma)
        {
            for (int j = 0; j < image.GetHeight(); ++j)
            {
                for (int i = 0; i < image.GetWidth(); ++i)
                {
                    bAvg += getPixelAVG(image, i, j);
                }
            }
            fAvg = (float)(bAvg / (image.GetWidth() * image.GetHeight())) / 255.f;
            fAvg *= 1.35f;
        }

        for (int j = 0; j < sh; ++j)
        {
            for (int i = 0; i < sw; ++i)
            {
                auto ix = (int)((float)i * ratio);
                auto iy = (int)((float)j * ratioh);
                auto toix = (int)((float)(i + 1) * ratio);
                auto toiy = (int)((float)(j + 1) * ratioh);

                if (setting_multi)
                {
                    auto sizex = toix - ix;
                    auto sizey = toiy - iy;

                    float b[9] = {0};

                    // Top left
                    {
                        unsigned long avg = 0, cnt = 0;
                        if (setting_filter)
                        {
                            for (int jj = iy; jj < iy + max<>(1, (sizey * 3 / 12)); ++jj)
                                for (int ii = ix; ii < ix + max<>(1, (sizex * 2 / 8)); ++ii)
                                {
                                    avg += getPixelAVG(image, ii, jj);
                                    ++cnt;
                                }
                            if (cnt) avg /= cnt;
                        }
                        else
                        {
                            avg = getPixelAVG(image, ix, iy);
                        }
                        b[0] = (float)avg / 255.f;
                    }

                    // Top
                    {
                        unsigned long avg = 0, cnt = 0;
                        if (setting_filter)
                        {
                            for (int jj = iy; jj < iy + max<>(1, (sizey * 3 / 12)); ++jj)
                                for (int ii = ix + max<>(1, (sizex * 2 / 8)); ii < ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)); ++ii)
                                {
                                    avg += getPixelAVG(image, ii, jj);
                                    ++cnt;
                                }
                            if (cnt) avg /= cnt;
                        }
                        else
                        {
                            avg = getPixelAVG(image, ix + max<>(1, (sizex * 2 / 8)), iy);
                        }
                        b[1] = (float)avg / 255.f;
                    }

                    // Top right
                    {
                        unsigned long avg = 0, cnt = 0;
                        if (setting_filter)
                        {
                            for (int jj = iy; jj < iy + max<>(1, (sizey * 3 / 12)); ++jj)
                                for (int ii = ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)); ii < ix + sizex; ++ii)
                                {
                                    avg += getPixelAVG(image, ii, jj);
                                    ++cnt;
                                }
                            if (cnt) avg /= cnt;
                        }
                        else
                        {
                            avg = getPixelAVG(image, ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)), iy);
                        }
                        b[2] = (float)avg / 255.f;
                    }

                    // Left
                    {
                        unsigned long avg = 0, cnt = 0;
                        if (setting_filter)
                        {
                            for (int jj = iy + max<>(1, (sizey * 3 / 12)); jj < iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)); ++jj)
                                for (int ii = ix; ii < ix + max<>(1, (sizex * 2 / 8)); ++ii)
                                {
                                    avg += getPixelAVG(image, ii, jj);
                                    ++cnt;
                                }
                            if (cnt) avg /= cnt;
                        }
                        else
                        {
                            avg = getPixelAVG(image, ix, iy + max<>(1, (sizey * 3 / 12)));
                        }
                        b[3] = (float)avg / 255.f;
                    }

                    // Center
                    {
                        unsigned long avg = 0, cnt = 0;
                        if (setting_filter)
                        {
                            for (int jj = iy + max<>(1, (sizey * 2 / 12)); jj < iy + max<>(1, (sizey * 2 / 12)) + max<>(1, (sizey * 8 / 12)); ++jj)
                                for (int ii = ix + max<>(1, (sizex * 1 / 8)); ii < ix + max<>(1, (sizex * 1 / 8)) + max<>(1, (sizex * 6 / 8)); ++ii)
                                {
                                    avg += getPixelAVG(image, ii, jj);
                                    ++cnt;
                                }
                            if (cnt) avg /= cnt;
                        }
                        else
                        {
                            avg = getPixelAVG(image, ix + max<>(1, (sizex * 2 / 8)), iy + max<>(1, (sizey * 3 / 12)));
                        }
                        b[4] = (float)avg / 255.f;
                    }

                    // Right
                    {
                        unsigned long avg = 0, cnt = 0;
                        if (setting_filter)
                        {
                            for (int jj = iy + max<>(1, (sizey * 3 / 12)); jj < iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)); ++jj)
                                for (int ii = ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)); ii < ix + sizex; ++ii)
                                {
                                    avg += getPixelAVG(image, ii, jj);
                                    ++cnt;
                                }
                            if (cnt) avg /= cnt;
                        }
                        else
                        {
                            avg = getPixelAVG(image, ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)), iy + max<>(1, (sizey * 3 / 12)));
                        }
                        b[5] = (float)avg / 255.f;
                    }

                    // Bottom left
                    {
                        unsigned long avg = 0, cnt = 0;
                        if (setting_filter)
                        {
                            for (int jj = iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)); jj < iy + sizey; ++jj)
                                for (int ii = ix; ii < ix + max<>(1, (sizex * 2 / 8)); ++ii)
                                {
                                    avg += getPixelAVG(image, ii, jj);
                                    ++cnt;
                                }
                            if (cnt) avg /= cnt;
                        }
                        else
                        {
                            avg = getPixelAVG(image, ix, iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)));
                        }
                        b[6] = (float)avg / 255.f;
                    }

                    // Bottom
                    {
                        unsigned long avg = 0, cnt = 0;
                        if (setting_filter)
                        {
                            for (int jj = iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)); jj < iy + sizey; ++jj)
                                for (int ii = ix + max<>(1, (sizex * 2 / 8)); ii < ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)); ++ii)
                                {
                                    avg += getPixelAVG(image, ii, jj);
                                    ++cnt;
                                }
                            if (cnt) avg /= cnt;
                        }
                        else
                        {
                            avg = getPixelAVG(image, ix + max<>(1, (sizex * 2 / 8)), iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)));
                        }
                        b[7] = (float)avg / 255.f;
                    }

                    // Bottom right
                    {
                        unsigned long avg = 0, cnt = 0;
                        if (setting_filter)
                        {
                            for (int jj = iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)); jj < iy + sizey; ++jj)
                                for (int ii = ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)); ii < ix + sizex; ++ii)
                                {
                                    avg += getPixelAVG(image, ii, jj);
                                    ++cnt;
                                }
                            if (cnt) avg /= cnt;
                        }
                        else
                        {
                            avg = getPixelAVG(image, ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)), iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)));
                        }
                        b[8] = (float)avg / 255.f;
                    }

                    if (setting_gamma)
                    {
                        for (auto &_b : b)
                        {
                            _b = pow(_b, 1.f + (fAvg - .5f));
                        }
                    }

                    float bestScore = 10000.f;
                    char c = ' ';
                    for (auto &ch : chars)
                    {
                        auto score = getScore(ch, b);
                        if (score < bestScore)
                        {
                            c = ch.c;
                            bestScore = score;
                        }
                    }

                    cout << c;
                }
                else
                {
                    unsigned long avg = 0, cnt = 0;
                    if (setting_filter)
                    {
                        for (int jj = iy; jj < toiy; ++jj)
                        {
                            for (int ii = ix; ii < toix; ++ii)
                            {
                                avg += getPixelAVG(image, ii, jj);
                                ++cnt;
                            }
                        }
                        if (cnt) avg /= cnt;
                    }
                    else
                    {
                        avg = getPixelAVG(image, ix, iy);
                    }

                    auto B = (float)avg / 255.f;

                    // Easier to differentiate difference in lower brightness
                    if (setting_gamma)
                    {
                        B = pow(B, 1.f + (fAvg - .5f));
                    }

                    auto& mm = bToC;
                    if (setting_simple)
                    {
                        mm = bToC_simple;
                    }
                    else if (setting_random)
                    {
                        mm = bToC_full;
                    }

                    char c = ' ';
                    auto ret = mm.equal_range(B);
                    auto l = ret.first;
                    auto u = ret.second;
                    if (l != mm.begin()) l--;
                    if (u == mm.end() || B - l->first < u->first - B)
                    {
                        c = l->second[rand() % l->second.size()];
                    }
                    else
                    {
                        c = u->second[rand() % u->second.size()];
                    }

                    cout << c;
                }
            }
            cout << endl;
        }
    }
    else
    {
        cout << "result = " << status << endl;
        cout << "failed" << endl;
    }

    // Uncomment for easier debug from VS
    //system("pause");

    return 0;
}
