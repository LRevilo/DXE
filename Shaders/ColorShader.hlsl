
cbuffer GlobalBuffer : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix ViewProjectionMatrix;

    float3 CameraPosition;
    float DeltaTime;

    float3 SunColor;
    float Time;

    float3 SunDirection;
    float SunIntensity;

    float3 AmbientLight;
    int FrameCount;

    float2 ScreenSize;
    float2 MousePosition;
}

cbuffer Material : register(b1)
{
    float4 Colour;
}


struct InstanceInput
{
    float4x4 world : INSTANCE_TRANSFORM; // Instance transform matrix
    float4 color : INSTANCE_COLOR; // Instance colour
};

// Vertex structure
struct VertexInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};


// Output to pixel shader
struct VertexOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};



// **Vertex Shader with Instancing**
VertexOutput vs_main(VertexInput vin, InstanceInput inst, uint instanceID : SV_InstanceID)
{
    VertexOutput vout;


    float4 colour = inst.color;
    float4 worldPos = mul(float4(vin.pos, 1.0f), inst.world);
    
    
    //float4 worldPos = mul(float4(vin.pos, 1.0f), identityMatrix);
    
    vout.pos = mul(worldPos, ViewProjectionMatrix); // Convert to clip space
    vout.uv = vin.uv;
    vout.color = colour; // Multiply instance color with vertex color

    return vout;
}

// **Pixel Shader**
float4 ps_main(VertexOutput pin) : SV_TARGET
{
    return pin.color;
}
