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

cbuffer Time : register(b4) {
	float deltaTime;
}

static const float PI = 3.1415926535f;
static const float delta = 0.1 / 180.0f * PI;
float3 GetTwistPosition(float t, float radius, float y) {
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

float4x4 Scale(float3 scale, float4x4 mat) {
	return float4x4(
		mat[0] * scale.x,
		mat[1] * scale.y,
		mat[2] * scale.z,
		mat[3]
		);
}

float4x4 GetOrthonormalBasis(float3 x, float3 y, float3 z) {
	return float4x4(
		float4(x.x, x.y, x.z, 0.0f),
		float4(y.x, y.y, y.z, 0.0f),
		float4(z.x, z.y, z.z, 0.0f),
		float4(0.0f, 0.0f, 0.0f, 1.0f)
		);
}

float4x4 GetMatrix(float3 x, float3 y, float3 z, float3 position, float t) {
	return float4x4(
		float4(x.x, x.y, x.z, 0.0f),
		float4(y.x, y.y, y.z, 0.0f),
		float4(z.x, z.y, z.z, 0.0f),
		float4(position + float3(y.x,y.y,y.z) * -t, 1.0f)
		);
}

float ConvToRadian(float degree) {
	return degree / 180.0f * PI;
}

static const float maxHeight = 144.0f * 0.25f;

float Loop() {
	return sin(ConvToRadian(deltaTime / 3.6f));
}

float NormLoop() {
	return (Loop() + 1.0f) * 0.5f;
}

float4x4 GetTwistMatrix(float t) {
	float rad = t / 180.0f * PI * 20.0f * NormLoop();
	float radius = 2.0f;
	float3 position = GetTwistPosition(rad, radius,t);
	float3 deltaPos = GetTwistPosition(rad + delta, radius, t + 0.1f);
	float max = maxHeight;
	float heightRate = 1 - t / max;
	//heightRate *= 2.5f;
	//heightRate = clamp(heightRate, 0.0f, 1.0f);
	//heightRate *= heightRate;

	float3 y = normalize(deltaPos - position);
	float3 z = float3(0.0f, -1.0f, 0.0f);
	float3 x = normalize(cross(y, z));
	z = cross(x, y);
	position.y = t * NormLoop();
	float4x4 inv = Translate(float3(0.0f, -t, 0.0f));
	return mul(mul(inv, Scale(float3(heightRate,1.0f, heightRate),GetOrthonormalBasis(x, y, z))),Translate(position));
}

PSInput main(float3 pos : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD) {
	PSInput input;
	float4 worldPosition = mul(float4(pos, 1.0f), GetTwistMatrix(pos.y));
	//worldPosition = float4(pos, 1.0f);
	input.position = mul(mul(worldPosition, view), projection);
	float3 lightPos = float3(0.0f, 60.0f, 0.0f);
	float3 lightVec = worldPosition.xyz - lightPos;
	float dist = 1.0f / sqrt(dot(lightVec, lightVec));
	float4 lightColor = float4(1.0f, 1.0f, 1.0f, 0.0f) * 100.0f;
	input.color = color;// *dist * lightColor;

	input.texcoord = texcoord;
	return input;
}