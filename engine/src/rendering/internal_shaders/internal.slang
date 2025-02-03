StructuredBuffer<float> buffer0;
StructuredBuffer<float> buffer1;
RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    uint index = threadId.x;
    result[index] = buffer0[index] + buffer1[index];
}

struct VOutTest
{
    float4 position : SV_Position;
    float3 color : COLOR;
}

struct VOutDedicatedSprite
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
}

struct DedicatedSpriteConstats
{
    float2 spritePos;
    float2 spriteSize;
    float spriteAngle;
};

[vk::push_constant]
ConstantBuffer<DedicatedSpriteConstats> dedicatedSpriteConstants : register(b0);

float2x2 createRotationMatrix2x2(float angle)
{
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    return float2x2(cosTheta, -sinTheta, // First column
                    sinTheta, cosTheta   // Second column
    );
}

[shader("vertex")]
VOutDedicatedSprite dedicatedSpriteVS(uint id: SV_VertexID)
{
    float2 positions[4] = {
        float2(-0.5f, -0.5f),
        float2(0.5f, -0.5f),
        float2(-0.5f, 0.5f),
        float2(0.5f, 0.5f),
    };

    VOutDedicatedSprite output;

    float2 canvasPos = mul(positions[id] * dedicatedSpriteConstants.spriteSize,
                           createRotationMatrix2x2(dedicatedSpriteConstants.spriteAngle)) +
                       dedicatedSpriteConstants.spritePos;

    canvasPos *= float2(1.0f / 400.f, 1.0f / 300.0f);
    canvasPos -= float2(1.0f, 1.0f);

    output.position = float4(canvasPos, 0.0f, 1.0f);
    output.color = float4(0.9, 0.5, 0.2, 1.0);
    output.uv = positions[id] + float2(0.5f, 0.5f);
    return output;
}

[shader("vertex")]
VOutTest testTriangleVS(uint id: SV_VertexID)
{
    VOutTest output;
    float2 positions[3] = {
        float2(0.0f, 0.5f),  // Top vertex
        float2(0.5f, -0.5f), // Bottom-right vertex
        float2(-0.5f, -0.5f) // Bottom-left vertex
    };

    // Assign the position based on the vertex ID
    output.position = float4(positions[id], 0.0f, 1.0f);

    // Assign a color based on the vertex ID
    float3 colors[3] = {
        float3(1.0f, 0.0f, 0.0f), // Red
        float3(0.0f, 1.0f, 0.0f), // Green
        float3(0.0f, 0.0f, 1.0f)  // Blue
    };

    output.color = colors[id];
    return output;
}

[shader("fragment")]
float4 testTriangleFS(VOutTest input) : SV_Target
{
    return float4(input.color, 1.0f);
}

[shader("fragment")]
float4 dedicatedSpriteFS(VOutDedicatedSprite input) : SV_Target
{
    return input.color;
}
