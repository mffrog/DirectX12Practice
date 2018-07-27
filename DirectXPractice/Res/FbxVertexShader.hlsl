struct PSInput {
	float4 position : SV_POSITION;
	float4 worldPosition : WORLD_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
};

cbuffer ViewMat : register(b0) {
	float4x4 view;
}

cbuffer ProjMat : register(b1) {
	float4x4 projection;
}

cbuffer WorldMat : register(b2) {
	float4x4 world;
}

PSInput main( float3 pos : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD, float3 normal : NORMAL ) 
{
	PSInput input;
	input.worldPosition = mul(float4(pos, 1.0f), world);
	input.position = mul(mul(input.worldPosition, view), projection);
	input.color = color;
	input.texcoord = texcoord;
	input.normal = normal;
	return input;
}