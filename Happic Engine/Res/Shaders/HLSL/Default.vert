struct VSInput
{
	float3 Pos : POSITION;
	float2 TexCoord : TEXCOORD;
};

struct VSOutput
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

cbuffer PerDrawInstance
{
	float4x4 mvp;
};

VSOutput main(VSInput input)
{

	VSOutput output;
	output.Pos = mul(float4(input.Pos, 1.0), transpose(mvp));
	output.TexCoord = input.TexCoord;
	return output;
}