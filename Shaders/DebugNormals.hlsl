cbuffer GlobalBuffer : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix ViewProjectionMatrix;

    matrix LightViewMatrix;
    matrix LightProjectionMatrix;
    matrix LightViewProjectionMatrix;
   
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
    float3 PlayerPos;
    float padding;
};

struct InstanceInput
{
    float4x4 world : INSTANCE_TRANSFORM; // Instance transform matrix
    float4 color : INSTANCE_COLOR; // Instance colour
};


struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    
    float4 row0 : TEXCOORD2;
    float4 row1 : TEXCOORD3;
    float4 row2 : TEXCOORD4;
    float4 row3 : TEXCOORD5;
};

VSOutput vs_main(VSInput vin, InstanceInput inst, uint instanceID : SV_InstanceID)
{
    VSOutput vout;
    vout.pos = vin.pos;
    vout.normal = vin.normal;
    
    // Copy each row of the matrix
    vout.row0 = inst.world[0];
    vout.row1 = inst.world[1];
    vout.row2 = inst.world[2];
    vout.row3 = inst.world[3];
    
    return vout;
}

struct GSOutput
{
    float4 pos : SV_POSITION;
    float3 col : COLOR;
};

[maxvertexcount(2)]
void gs_main(point VSOutput input[1], inout LineStream<GSOutput> lineStream)
{
    float4x4 world = float4x4(input[0].row0, input[0].row1, input[0].row2, input[0].row3);
    
    
    float3 startWorld = mul(float4(input[0].pos, 1.0f), world).xyz;

    // Transform normal with upper 3x3 of world matrix (ignore translation)
    float3x3 normalMatrix = (float3x3) world;
    float3 normalWorld = mul(input[0].normal, normalMatrix);
    normalWorld = normalize(normalWorld);

    float3 endWorld = startWorld + normalWorld * 0.2f;

    GSOutput v;

    // Start point
    v.pos = mul(float4(startWorld, 1.0f), ViewProjectionMatrix);
    v.col = float3(1, 1, 1); // white
    lineStream.Append(v);

    // End point
    v.pos = mul(float4(endWorld, 1.0f), ViewProjectionMatrix);
    v.col = float3(1, 0, 0); // red
    lineStream.Append(v);
}


float4 ps_main(GSOutput pin) : SV_TARGET
{
    return float4(pin.col, 1.0f);
}