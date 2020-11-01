struct VS_OUTPUT {
	float4 pos: SV_POSITION;
	float4 color: COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
	float4 colorMultiplier;
}

VS_OUTPUT main(float3 pos: POSITION, float4 color : COLOR) {
	VS_OUTPUT output;
	output.pos = float4(pos, 1.);
	output.color = color * colorMultiplier;
	return output;
}

