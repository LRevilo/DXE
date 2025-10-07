
cbuffer MatrixBuffer : register(b0)
{
    float testFloat;
    float3 padding;
    matrix TestMat; // View * Projection matrix
    matrix viewProj; // View * Projection matrix
}

cbuffer TransformBuffer : register(b1)
{
    float4x4 world; // Transform matrix
    float4 color; // Colour
    int instances;
}

// Instance structure (matches InstanceData struct in C++)
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

static const float4x4 identityMatrix = float4x4(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);


// **Vertex Shader with Instancing**
VertexOutput vs_main(VertexInput vin, InstanceInput inst, uint instanceID : SV_InstanceID)
{
    VertexOutput vout;
    float4x4 posMat = identityMatrix;
    // Transform vertex position using world matrix from instance
    if (instanceID >= 0)
    {
        posMat = inst.world;
    }

    //float4x4 posMat = (instanceID > 0) ? inst.world : identityMatrix;
    float4 colour = (instanceID > 0) ? vin.color : vin.color;
    
    float4 worldPos = mul(float4(vin.pos, 1.0f), posMat);
    
    
    //float4 worldPos = mul(float4(vin.pos, 1.0f), identityMatrix);
    
    vout.pos = mul(worldPos, viewProj); // Convert to clip space
    vout.uv = vin.uv;
    vout.color = colour; // Multiply instance color with vertex color

    return vout;
}

// **Pixel Shader**
float4 ps_main(VertexOutput pin) : SV_TARGET
{
    return pin.color;
}
