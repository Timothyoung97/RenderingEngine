static float2 clipToScreenSpace(float2 xy) {
    return xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
}

static float2 screenToClipSpace(float2 xy, float2 viewportDimension) {
    return (xy / viewportDimension) * float2(2.0f, -2.0) + float2(-1, 1);
}
