struct PSInput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};
cbuffer RCView : register(b0) {
	float4x4 view;
}

cbuffer RCProjection : register(b1) {
	float4x4 projection;
}

cbuffer RCWorld : register(b2) {
	float4x4 world;
}

PSInput main( float3 pos : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD )
{
	PSInput input;
	input.position = mul(mul(mul(float4(pos, 1.0f), world), view), projection);
	float4 ambient = float4(0.01f,0.01f,0.01f, 0.0f);
	float3 colorDist = pos - float3(50.0f, 30.0f, 0.0f);
	float dist = dot(colorDist, colorDist);
	dist *= dist;
	float3 colorPower = float3(1.0f,1.0f,1.0f) * 2500000;
	input.color = color;
	input.color.rgb *= colorPower / dist;
	input.texcoord = texcoord;
	return input;
}