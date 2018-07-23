Texture2D tex0 : register(t0);
SamplerState sampler0 : register(s0);

struct PSInput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
	return input.color;
}