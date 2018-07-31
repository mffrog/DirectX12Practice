struct PSInput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
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

static const float PI = 3.1415926535f;
static const float delta = 0.1 / 180.0f * PI;
float3 GetTwistPosition(float t,float radius, float y) {
	return float3(cos(t) * radius, y, sin(t) * radius);
}

float4x4 Translate(float3 pos) {
	return float4x4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		pos.x, pos.y, pos.z, 1.0f
		);
}

float3 GetOri(float t) {
	float rad = t / 180.0f * PI * 5.0f;
	float radius = 10.0f;
	float3 position = GetTwistPosition(rad, radius,t);
	float3 deltaPos = GetTwistPosition(rad + delta, radius,t);

	float3 y = normalize(deltaPos - position);
	float3 z = float3(0.0f, -1.0f, 0.0f);
	float3 x = normalize(cross(y, z));
	z = cross(x, y);
	return z;
}

float4x4 GetMatrix(float3 x, float3 y, float3 z, float3 position, float t) {
	return float4x4(
		float4(x.x, x.y, x.z, 0.0f),
		float4(y.x, y.y, y.z, 0.0f),
		float4(z.x, z.y, z.z, 0.0f),
		float4(position, 1.0f)
		//float4(0.0f,0.0f,0.0f,1.0f)
		);

	return float4x4(
		x.x, x.y, x.z, 0.0f,
		y.x, y.y, y.z, 0.0f,
		z.x, z.y, z.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);
}

float4x4 GetTwistMatrix(float t) {
	float rad = t / 180.0f * PI * 5.0f;
	//rad = 45.0f / 180.0f * PI;
	float radius = 10.0f;
	float3 position = GetTwistPosition(rad, radius,t);
	float3 deltaPos = GetTwistPosition(rad + delta, radius,t);

	float3 y = normalize(deltaPos - position);
	float3 z = float3(0.0f, -1.0f, 0.0f);
	float3 x = normalize(cross(y, z));
	z = cross(x, y);
	//position *= 10.0f;
	position.y = 0.0f;
	float4x4 inv = Translate(float3(0.0f, -t, 0.0f));
	return mul(inv, GetMatrix(x, y, z, position, t));
}

PSInput main(float3 pos : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD) {
	PSInput input;
	float4 worldPosition = mul(float4(pos, 1.0f), GetTwistMatrix(pos.y));
	//worldPosition = float4(pos, 1.0f);
	input.position = mul(mul(worldPosition, view), projection);
	float3 lightPos = float3(0.0f, 60.0f, 0.0f);
	float3 lightVec = worldPosition - lightPos;
	float dist = 1.0f / sqrt(dot(lightVec, lightVec));
	float4 lightColor = float4(1.0f, 1.0f, 1.0f, 0.0f) * 100.0f;
	input.color = color * dist * lightColor;

	float4 colorRes = float4(GetOri(pos.y), 1.0f);
	colorRes.xyz = (float3(1.0f, 1.0f, 1.0f) + colorRes.xyz) * 0.5f;
	input.color = colorRes;

	float4 colorPos = float4((pos + float3(1.0f, 1.0f, 1.0f)) * 0.5f, 1.0f);
	input.color = colorPos;

	float4 colorGet = float4((GetOri(pos.y) + float3(1.0f, 1.0f, 1.0f) * 0.5f), 1.0f);
	input.color = colorGet;

	input.texcoord = texcoord;
	return input;
}