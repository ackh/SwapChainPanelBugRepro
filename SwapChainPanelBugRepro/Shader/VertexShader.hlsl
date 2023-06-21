struct VertexShaderInput
{
  float3 Position : POSITION;
  float3 Color    : COLOR;
};


struct VertexShaderOutput
{
  float4 Position : SV_POSITION;
  float4 Color    : COLOR;
};


VertexShaderOutput main(VertexShaderInput input)
{
  VertexShaderOutput output;
  output.Color    = float4(input.Color, 1.0f);
  output.Position = float4(input.Position, 1.0f);
  return output;
}
