StructuredBuffer<float> buffer0;
StructuredBuffer<float> buffer1;
RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint index = threadId.x;
    result[index] = buffer0[index] + buffer1[index];
}

struct VOut
{
    float4 position : SV_Position;
    float3 color : COLOR;
}

[shader("vertex")]
VOut testTriangleVS(uint id: SV_VertexID)
{
    VOut output;
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
float4 testTriangleFragment(VOut input) : SV_Target
{
    return float4(input.color,1.0f);
}