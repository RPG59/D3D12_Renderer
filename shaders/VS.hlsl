struct VS_OUTPUT {
	float4 pos: SV_POSITION;
	float4 color: COLOR;
};

VS_OUTPUT main(float3 pos: POSITION, float4 color : COLOR) {
	VS_OUTPUT output;
	output.pos = float4(pos, 1.);
	output.color = color;
	return output;
}

