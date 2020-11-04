// constant buffers -> b redister
// textures -> t register
// us -> u register

struct VS_OUTPUT {
	float4 pos: SV_POSITION;
	float4 color: COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 mvpMatrix;
}

VS_OUTPUT main(float3 pos: POSITION, float4 color : COLOR) {
	VS_OUTPUT output;
	output.pos = mul(float4(pos, 1.), mvpMatrix);
	output.color = color;
	return output;
}

