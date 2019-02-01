struct VSOutput
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

SamplerState MSamp : register(s0);
Texture2D Albedo : register(t0);

float4 main(VSOutput input) : SV_TARGET0
{
	return Albedo.Sample(MSamp, input.TexCoord);
}