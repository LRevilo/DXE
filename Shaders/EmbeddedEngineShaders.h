#pragma once 
#include <string> 
#include <unordered_map> 
 
namespace DXE { 
    static const std::unordered_map<std::string, std::string> RawEngineShaderMap = { 
        {  "BlinnPhong.hlsl", R"( 
// " 
cbuffer vsConstants : register(b0)
{
    float4x4 modelViewProj;
    float4x4 modelView;
    float3x3 normalMatrix;
};

struct DirectionalLight
{
    float4 dirEye; //NOTE: Direction *towards* the light
    float4 color;
};

struct PointLight
{
    float4 posEye;
    float4 color;
};

// Create Constant Buffer for our Blinn-Phong vertex shader
cbuffer fsConstants : register(b0)
{
    DirectionalLight dirLight;
    PointLight pointLights[2];
};

struct VS_Input {
    float3 pos : POS;
    float2 uv : TEX;
    float3 norm : NORM;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float3 posEye : POSITION;
    float3 normalEye : NORMAL;
    float2 uv : TEXCOORD;
};

Texture2D    mytexture : register(t0);
SamplerState mysampler : register(s0);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(float4(input.pos, 1.0f), modelViewProj);
    output.posEye = mul(float4(input.pos, 1.0f), modelView).xyz;
    output.normalEye = mul(input.norm, normalMatrix);
    output.uv = input.uv;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float3 diffuseColor = mytexture.Sample(mysampler, input.uv).xyz;

    float3 fragToCamDir = normalize(-input.posEye);
    
    // Directional Light
    float3 dirLightIntensity;
    {
        float ambientStrength = 0.1;
        float specularStrength = 0.9;
        float specularExponent = 100;
        float3 lightDirEye = dirLight.dirEye.xyz;
        float3 lightColor = dirLight.color.xyz;

        float3 iAmbient = ambientStrength;

        float diffuseFactor = max(0.0, dot(input.normalEye, lightDirEye));
        float3 iDiffuse = diffuseFactor;

        float3 halfwayEye = normalize(fragToCamDir + lightDirEye);
        float specularFactor = max(0.0, dot(halfwayEye, input.normalEye));
        float3 iSpecular = specularStrength * pow(specularFactor, 2*specularExponent);

        dirLightIntensity = (iAmbient + iDiffuse + iSpecular) * lightColor;
    }
    // Point Light
    float3 pointLightIntensity = float3(0,0,0);
    for(int i=0; i<2; ++i)
    {
        float ambientStrength = 0.1;
        float specularStrength = 0.9;
        float specularExponent = 100;
        float3 lightDirEye = pointLights[i].posEye.xyz - input.posEye;
        float inverseDistance = 1 / length(lightDirEye);
        lightDirEye *= inverseDistance; //normalise
        float3 lightColor = pointLights[i].color.xyz;

        float3 iAmbient = ambientStrength;

        float diffuseFactor = max(0.0, dot(input.normalEye, lightDirEye));
        float3 iDiffuse = diffuseFactor;

        float3 halfwayEye = normalize(fragToCamDir + lightDirEye);
        float specularFactor = max(0.0, dot(halfwayEye, input.normalEye));
        float3 iSpecular = specularStrength * pow(specularFactor, 2*specularExponent);

        pointLightIntensity += (iAmbient + iDiffuse + iSpecular) * lightColor * inverseDistance;
    }

    float3 result = (dirLightIntensity + pointLightIntensity) * diffuseColor;

    return float4(result, 1.0);
}
)" }, 
        {  "ColorShader.hlsl", R"( 

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
)" }, 
        {  "DebugNormals.hlsl", R"( 
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
})" }, 
        {  "Lights.hlsl", R"( 

cbuffer constants : register(b0)
{
    float4x4 modelViewProj;
    float4 color;
};

struct VertexShaderInput {
    float3 pos : POS;
};

struct VertexShaderOutput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VertexShaderOutput vs_main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.pos = mul(float4(input.pos, 1.0f), modelViewProj);
    output.color = color;
    return output;
}

float4 ps_main(VertexShaderOutput input) : SV_Target
{
    return input.color;
}
)" }, 
        {  "TestShader.hlsl", R"( 

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
)" }, 
        {  "Texture.hlsl", R"( 

cbuffer vsConstants : register(b0)
{
    float4x4 modelViewProj;
    float4x4 modelView;
    float3x3 normalMatrix;
};

struct DirectionalLight
{
    float4 dirEye; //NOTE: Direction *towards* the light
    float4 color;
};

struct PointLight
{
    float4 posEye;
    float4 color;
};

// Create Constant Buffer for our Blinn-Phong vertex shader
cbuffer fsConstants : register(b0)
{
    DirectionalLight dirLight;
    PointLight pointLights[2];
};

struct VS_Input {
    float3 pos : POS;
    float2 uv : TEX;
    float3 norm : NORM;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float3 posEye : POSITION;
    float3 normalEye : NORMAL;
    float2 uv : TEXCOORD;
};

Texture2D    mytexture : register(t0);
SamplerState mysampler : register(s0);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(float4(input.pos, 1.0f), modelViewProj);
    output.posEye = mul(float4(input.pos, 1.0f), modelView).xyz;
    output.normalEye = mul(input.norm, normalMatrix);
    output.uv = input.uv;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float3 diffuseColor = mytexture.Sample(mysampler, input.uv).xyz;

    float3 fragToCamDir = normalize(-input.posEye);
    
    // Directional Light
    float3 dirLightIntensity;
    {
        float ambientStrength = 0.1;
        float specularStrength = 0.9;
        float specularExponent = 100;
        float3 lightDirEye = dirLight.dirEye.xyz;
        float3 lightColor = dirLight.color.xyz;

        float3 iAmbient = ambientStrength;

        float diffuseFactor = max(0.0, dot(input.normalEye, lightDirEye));
        float3 iDiffuse = diffuseFactor;

        float3 halfwayEye = normalize(fragToCamDir + lightDirEye);
        float specularFactor = max(0.0, dot(halfwayEye, input.normalEye));
        float3 iSpecular = specularStrength * pow(specularFactor, 2*specularExponent);

        dirLightIntensity = (iAmbient + iDiffuse + iSpecular) * lightColor;
    }
    // Point Light
    float3 pointLightIntensity = float3(0,0,0);
    for(int i=0; i<2; ++i)
    {
        float ambientStrength = 0.1;
        float specularStrength = 0.9;
        float specularExponent = 100;
        float3 lightDirEye = pointLights[i].posEye.xyz - input.posEye;
        float inverseDistance = 1 / length(lightDirEye);
        lightDirEye *= inverseDistance; //normalise
        float3 lightColor = pointLights[i].color.xyz;

        float3 iAmbient = ambientStrength;

        float diffuseFactor = max(0.0, dot(input.normalEye, lightDirEye));
        float3 iDiffuse = diffuseFactor;

        float3 halfwayEye = normalize(fragToCamDir + lightDirEye);
        float specularFactor = max(0.0, dot(halfwayEye, input.normalEye));
        float3 iSpecular = specularStrength * pow(specularFactor, 2*specularExponent);

        pointLightIntensity += (iAmbient + iDiffuse + iSpecular) * lightColor * inverseDistance;
    }

    float3 result = (dirLightIntensity + pointLightIntensity) * diffuseColor;

    return float4(result, 1.0);
}
)" }, 
        {  "TextureShader.hlsl", R"( 

cbuffer constants : register(b0)
{
    float4x4 modelViewProj;
    float4 color;
};

struct VertexShaderInput {
    float3 pos : POS;
};

struct VertexShaderOutput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VertexShaderOutput vs_main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.pos = mul(float4(input.pos, 1.0f), modelViewProj);
    output.color = color;
    return output;
}

float4 ps_main(VertexShaderOutput input) : SV_Target
{
    return input.color;
}
)" }, 
    }; 
} 
