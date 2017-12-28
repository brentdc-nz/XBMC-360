//-------------------------------------------------------------------------------------
// Vertex shader
// We use the register semantic here to directly define the input register
// matWVP.  Conversely, we could let the HLSL compiler decide and check the
// constant table.
//-------------------------------------------------------------------------------------
const char* g_strVertexShaderProgram = 
" float4x4 matWVP : register(c0);              "  
"                                              "  
" struct VS_IN                                 "  
" {                                            " 
"     float4 ObjPos   : POSITION;              "  // Object space position 
"     float2 Uv    : TEXCOORD0;                 "  // Vertex color                 
" };                                           " 
"                                              " 
" struct VS_OUT                                " 
" {                                            " 
"     float4 ProjPos  : POSITION;              "  // Projected space position 
"     float2 Uv    : TEXCOORD0;                 "  
" };                                           "  
"                                              "  
" VS_OUT main( VS_IN In )                      "  
" {                                            "  
"     VS_OUT Out;                              "  
"     Out.ProjPos = mul( matWVP, In.ObjPos );  "  // Transform vertex into
"     Out.Uv = In.Uv;                    "  // Projected space and 
"     return Out;                              "  // Transfer color
" }                                            ";

//-------------------------------------------------------------------------------------
// Pixel shader
//-------------------------------------------------------------------------------------
const char* g_strPixelShaderProgram = 
" sampler2D  YTexture : register( s0 );			"
" sampler2D  UTexture : register( s1 );			"
" sampler2D  VTexture : register( s2 );			"
" struct PS_IN                                 "
" {                                            "
"     float2 Uv : TEXCOORD0;                    "  // Interpolated color from                      
" };                                           "  // the vertex shader
"                                              "  
" float4 main( PS_IN In ) : COLOR              "  
" {                                            " 
"												"
"		float4 Y_4D = tex2D( YTexture, In.Uv );  "
"		float4 U_4D = tex2D( UTexture, In.Uv );  "
"		float4 V_4D = tex2D( VTexture, In.Uv );  "
"                                             "
"		float R = 1.164 * ( Y_4D.r - 0.0625 ) + 1.596 * ( V_4D.r - 0.5 ); "
"		float G = 1.164 * ( Y_4D.r - 0.0625 ) - 0.391 * ( U_4D.r - 0.5 ) - 0.813 * ( V_4D.r - 0.5 ); "
"		float B = 1.164 * ( Y_4D.r - 0.0625 ) + 2.018 * ( U_4D.r - 0.5 );          "                
"                            "                 
"		float4 ARGB;     "                        
"		ARGB.a = 1.0;     "                       
"		ARGB.r = R;        "                      
"		ARGB.g = G;         "                     
"		ARGB.b = B;          "                    
"                            "                 
"		return ARGB;  "
"					"		
" }                                            "; 
