module ermy_engine;

public struct ErmyFrame
{
	float4x4 ViewMatrix[2];
	float4x4 ProjMatrix[2];
	float4x4 ViewProjMatrix[2];

	float4x4 ViewMatrixInv[2];
	float4x4 ProjMatrixInv[2];
	float4x4 ViewProjMatrixInv[2];

	float2x2 canvasPreRotate;
	float4 canvasClearColor;

	float3 CameraWorldPos[2];
	float3 CameraViewDir[2];

	float2 canvasSize;

	float time;
	float deltaTime;
	float refreshRateTime;
};