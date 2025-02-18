struct PreviewTextureConstants2D
{
    float2 uv0;
    float2 uv1;
};

struct PreviewTextureConstantsCubemap
{
    float4 dir_fov;
};

struct StaticVertexDedicated
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float4 color : COLOR0;
};

struct OutStaticVertexDedicated
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct VOutFS
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

[shader("vertex")]
OutStaticVertexDedicated dedicatedStaticMeshVS(StaticVertexDedicated vtx, uniform float4x4 MVP)
{
    OutStaticVertexDedicated output;

    // Transform the vertex position using the MVP matrix
    output.position = mul(MVP, float4(vtx.pos, 1.0));

    // Pass through the UV coordinates
    output.uv = vtx.uv0;

    // Pass through the vertex color
    output.color = vtx.color;

    return output;
}

[shader("fragment")]
float4 dedicatedStaticMeshFS_UV0(OutStaticVertexDedicated input) : SV_Target
{
    return float4(input.uv.x,input.uv.y,0.0f,1.0f);
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

float3 checkerColor(float2 uv, float cellSize)
{
    // Estimate the screen resolution using derivatives of UV
    float2 pixelSize = float2(length(ddx(uv)), length(ddy(uv)));
    float2 resolution = 1.0 / pixelSize;

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
    return float4(checkerColor(input.uv,16.0f), 1.0);
}

[shader("fragment")]
float4 fullscreenFS2D(VOutFS input, uniform PreviewTextureConstants2D transform) : SV_Target
{
    float2 uv = lerp(transform.uv0, transform.uv1, input.uv);

    float4 color = preview2D.Sample(uv);
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        color.w = 0.0f;

    float3 bgColor = checkerColor(input.uv,16.0f);
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

[shader("fragment")]
float4 fullscreenFS2DStatic(VOutFS input) : SV_Target
{
    float4 color = preview2D.Sample(input.uv);
    float3 bgColor = checkerColor(input.uv,8.0f);
    return float4(lerp(bgColor, color.xyz, color.w), 1.0);
}

float3 CubeUVToDir(int faceID, float2 uv)
{
    uv.y = 1.0 - uv.y;
    uv = uv * 2.0 - 1.0; // Remap from [0,1] to [-1,1]
    
    float3 dir;

    if (faceID == 0) dir = float3(1, uv.y, -uv.x); // +X
    if (faceID == 1) dir = float3(-1, uv.y, uv.x); // -X
    if (faceID == 2) dir = float3(uv.x, 1, -uv.y); // +Y
    if (faceID == 3) dir = float3(uv.x, -1, uv.y); // -Y
    if (faceID == 4) dir = float3(uv.x, uv.y, 1); // +Z
    if (faceID == 5) dir = float3(-uv.x, uv.y, -1); // -Z

    return normalize(dir); // Normalize for correct cubemap sampling
}

[shader("fragment")]
float4 fullscreenFSCubemapStatic(VOutFS input) : SV_Target
{
    int rowID = (int)(input.uv.y * 3);
    int columnID = (int)(input.uv.x * 4);

    int faceID = -1;

    float3 color = checkerColor(input.uv, 8.0f);
    if (columnID == 0)
    {
        if (rowID == 1)
            faceID = 1;
    }
    if (columnID == 1)
    {
        if (rowID == 0)
            faceID = 2;
        if (rowID == 1)
            faceID = 4;
        if (rowID == 2)
            faceID = 3;
    }
    if (rowID == 1)
    {
        if (columnID == 2)
            faceID = 0;
        if (columnID == 3)
            faceID = 5;
    }
    if (faceID >= 0)
    {
        float3 dir = CubeUVToDir(faceID, fmod(input.uv * float2(4.0,3.0),float2(1.0f)));
        color = previewCube.Sample(dir).xyz;
    }

    return float4(color,1.0);
}