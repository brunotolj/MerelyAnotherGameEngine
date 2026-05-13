
struct VSInput
{
    float Index;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float2 EmPosition;
};

struct BezierCurve
{
    float x0;
    float y0;
    float x1;
    float y1;
    float x2;
    float y2;
};

struct PushConstants
{
    float4 Color;
    BezierCurve* Curves;
    int CurveCount;
    float ScreenPosX;
    float ScreenPosY;
    float ScreenSizeX;
    float ScreenSizeY;
    float GlyphBoundsMinX;
    float GlyphBoundsMinY;
    float GlyphBoundsMaxX;
    float GlyphBoundsMaxY;
};

[[vk::push_constant]] PushConstants push;

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    VSOutput output;

    float x = fmod(input.Index, 2.0);
    float y = 1.0 - floor(input.Index / 2.0);

    output.Position.x = push.ScreenPosX + push.ScreenSizeX * x;
    output.Position.y = push.ScreenPosY - push.ScreenSizeY * y;
    output.Position.z = 0.0;
    output.Position.w = 1.0;

    output.EmPosition.x = push.GlyphBoundsMinX + x * (push.GlyphBoundsMaxX - push.GlyphBoundsMinX);
    output.EmPosition.y = push.GlyphBoundsMinY + y * (push.GlyphBoundsMaxY - push.GlyphBoundsMinY);

    return output;
}

int calcWindingNumber(BezierCurve *curves, int curveCount, float x, float y)
{
    int winding = 0;

    for (int i = 0; i < curveCount; ++i)
    {
        BezierCurve curve = curves[i];
        if (y >= curve.y0 && y >= curve.y2) continue;
        if (y < curve.y0 && y < curve.y2) continue;

        if (x >= curve.x0 && x >= curve.x2) continue;
        if (x < curve.x0 && x < curve.x2)
        {
            winding += (curve.y2 < curve.y0) ? 1 : -1;
            continue;
        }

        float xa = curve.x0 + curve.x2 - 2.0 * curve.x1;
        float xb = 2.0 * (curve.x1 - curve.x0);
        float xc = curve.x0;

        float ya = curve.y0 + curve.y2 - 2.0 * curve.y1;
        float yb = 2.0 * (curve.y1 - curve.y0);
        float yc = curve.y0;

        if (ya > -0.0001 && ya < 0.0001)
        {
            float root = (y - curve.y0) / (curve.y2 - curve.y0);
            if (root >= 0.0 && root <= 1.0)
            {
                float pos = xa * root * root + xb * root + xc;
                if (x < pos)
                    winding += (curve.y2 < curve.y0) ? 1 : -1;
            }

            continue;
        }

        float d = yb * yb - 4.0 * ya * (yc - y);

        float rd = sqrt(d);

        float rootA = (-yb - rd) / (2.0 * ya);
        if (rootA >= 0.0 && rootA <= 1.0)
        {
            float pos = xa * rootA * rootA + xb * rootA + xc;
            if (x < pos)
                winding++;
        }

        float rootB = (-yb + rd) / (2.0 * ya);
        if (rootB >= 0.0 && rootB <= 1.0)
        {
            float pos = xa * rootB * rootB + xb * rootB + xc;
            if (x < pos)
                winding--;
        }
    }

    return winding;
}

[shader("fragment")]
float4 fragMain(VSOutput input, nointerpolation uint innerCoverage: SV_InnerCoverage) : SV_Target
{
    float x = input.EmPosition.x;
    float y = input.EmPosition.y;

    int winding = calcWindingNumber(push.Curves, push.CurveCount, x, y);
    if (winding > 0)
        return push.Color;

    discard;
    return float4(0.0, 0.0, 0.0, 0.0);
}
