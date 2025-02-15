//struct PreviewTextureConstants
//{
//    float2 uv0;
//    float2 uv1;
//};

//[vk::push_constant]
//ConstantBuffer<PreviewTextureConstants> previewTextureConst : register(b0);

struct VOutFS
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
}


[shader("vertex")]
VOutFS fullscreenVS(uint id: SV_VertexID)
{
    VOutFS output;

    // Fullscreen triangle vertex positions in clip space
    float2 positions[3] = {
        float2(-1.0f, -1.0f), // Bottom-left
        float2(3.0f, -1.0f),  // Bottom-right (extended beyond the screen)
        float2(-1.0f, 3.0f)   // Top-left (extended beyond the screen)
    };

    // Assign the position based on the vertex ID
    output.position = float4(positions[id], 0.0f, 1.0f);

    // Assign UV coordinates for fullscreen texture sampling
    float2 uvs[3] = {
        float2(0.0f, 1.0f), // Bottom-left
        float2(2.0f, 1.0f), // Bottom-right (extended beyond the screen)
        float2(0.0f, -1.0f) // Top-left (extended beyond the screen)
    };

    output.uv = float2(uvs[id].x, 1.0 - uvs[id].y);
    return output;
}

Sampler2D preview2D;

float3 checkerColor(float2 uv)
{
    // Estimate the screen resolution using derivatives of UV
    float2 pixelSize = float2(length(ddx(uv)), length(ddy(uv)));
    float2 resolution = 1.0 / pixelSize;

    // Define the checker size in pixels
    float cellSize = 16.0f;

    // Convert UV to approximate pixel coordinates
    float2 pixelCoords = uv * resolution;

    // Compute cell indices
    int2 cell = int2(floor(pixelCoords / cellSize));

    // Checkerboard pattern using XOR
    bool checker = (cell.x % 2) ^ (cell.y % 2);

    // Return black or white based on checkerboard pattern
    return checker ? float3(0.88, 0.89, 0.9) : float3(0.2, 0.19, 0.18);
}

[shader("fragment")]
float4 fullscreenFSEmpty(VOutFS input) : SV_Target
{
    return float4(checkerColor(input.uv), 1.0);
}

[shader("fragment")]
float4 fullscreenFS2D(VOutFS input) : SV_Target
{
    float4 color = preview2D.Sample(input.uv);
    return float4(lerp(checkerColor(input.uv), color.xyz, color.w), 1.0);
}