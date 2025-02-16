struct PreviewTextureConstants2D
{
    float2 uv0;
    float2 uv1;
};

struct PreviewTextureConstantsCubemap
{
    float4 dir_fov;
};

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

[vk_binding(0,0)]
Sampler2D preview2D;

[vk_binding(0, 0)]
SamplerCube previewCube;

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
float4 fullscreenFS2D(VOutFS input, uniform PreviewTextureConstants2D transform) : SV_Target
{
    float2 uv = lerp(transform.uv0, transform.uv1, input.uv);

    float4 color = preview2D.Sample(uv);
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        color.w = 0.0f;

    float3 bgColor = checkerColor(input.uv);
    return float4(lerp(bgColor, color.xyz, color.w), 1.0);
}

[shader("fragment")]
float4 fullscreenFSCubemap(VOutFS input, uniform PreviewTextureConstantsCubemap transform) : SV_Target
{
    float3 dir = transform.dir_fov.xyz;
    float fov = transform.dir_fov.w;

    float2 screenUV = input.uv * 2.0 - 1.0;

    float3 rayDir;
    rayDir.x = screenUV.x * fov;
    rayDir.y = screenUV.y * fov;
    rayDir.z = -1.0;

    rayDir = normalize(rayDir);

    float3 forward = normalize(dir);
    float3 right = normalize(cross(float3(0, 1, 0), forward));
    float3 up = cross(forward, right);

    float3 worldRayDir = rayDir.x * right + rayDir.y * up + rayDir.z * forward;

    return previewCube.Sample(worldRayDir);
}