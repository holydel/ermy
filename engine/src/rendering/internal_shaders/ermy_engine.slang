module ermy_engine;

public struct ErmyFrame
{
	public float4x4 ViewMatrix[2];
	public float4x4 ProjMatrix[2];
	public float4x4 ViewProjMatrix[2];

	public float4x4 ViewMatrixInv[2];
	public float4x4 ProjMatrixInv[2];
	public float4x4 ViewProjMatrixInv[2];

	public float4 canvasPreRotate; //float2x2 breaks alignment issue for uniform buffers
	public float4 canvasClearColor;

	public float4 CameraWorldPos[2];
	public float4 CameraViewDir[2];

    public float2 canvasRepSizeHalf; //float2(2.0 / width, 2.0 / height)

	public float time;
	public float deltaTime;
	public float refreshRateTime;
};

// Rotate a vector by a quaternion (assumes unit quaternion)
public float3 rotateVectorByQuaternion(float3 vec, float4 quat)
{
    float3 qvec = quat.xyz; // Vector part (x, y, z)
    float qw = quat.w;      // Scalar part (w)
    return vec + 2.0 * cross(qvec, cross(qvec, vec) + qw * vec);
}