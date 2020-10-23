struct VS_OUTPUT {
	float4 pos: SV_POSITION;
	float4 color: COLOR;
};

float4 main(VS_OUTPUT data) : SV_Target
{
	return data.color;
}

