struct PSInput {
	float4 position : SV_POSITION;
	float4 worldPosition : WORLD_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
};

cbuffer LightData : register(b3) {
	float4 lightPos;
	float4 lightColor;
}

float4 main(PSInput input) : SV_TARGET 
{
	float3 lightVec = -input.worldPosition.xyz + lightPos.xyz;
	float lightPower = 1 / dot(lightVec, lightVec);
	float cosTheta = clamp(dot(normalize(lightVec), input.normal),0.0f,1.0f);
	float3 lightResult = lightPower * cosTheta * lightColor.rgb;
	float4 resultColor = input.color;
	resultColor.rgb *= lightResult;
	return resultColor;
}