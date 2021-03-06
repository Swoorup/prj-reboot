//
//   HydraxShader9.hlsl for Ogre
//   Xavier Vergu�n Gonz�lez
//   December 2007
//

//   Refraction: Texture
//   Reflection: Texture
//   Additional Effects: Depth, Smooth transitions

void main_vp(// IN
             float4 iPosition         : POSITION,
             float2 iUv               : TEXCOORD0,
             // OUT
             out float4 oPosition     : POSITION,
             out float4 oPosition_    : TEXCOORD0,
             out float2 oUvNoise      : TEXCOORD1,
             out float4 oUvProjection : TEXCOORD2,
             // UNIFORM,
             uniform float4x4         uWorldViewProj)
{
    oPosition_ = iPosition;
	oPosition = mul(uWorldViewProj, iPosition);

	// Projective texture coordinates, adjust for mapping
	float4x4 scalemat = float4x4(0.5,   0,   0, 0.5,
	                               0,-0.5,   0, 0.5,
								   0,   0, 0.5, 0.5,
								   0,   0,   0,   1);

	oUvProjection = mul(scalemat, oPosition);
	oUvNoise = iUv;
}

// Expand a range-compressed vector
float3 expand(float3 v)
{
	return (v - 0.5) * 2;
}

void main_fp(// IN
             float4 iPosition     : TEXCOORD0,
             float2 iUvNoise      : TEXCOORD1,
             float4 iUvProjection : TEXCOORD2,
	         // OUT
	         out float4 oColor    : COLOR,
	         // UNIFORM
	         uniform float3       uEyePosition,
	         uniform float        uFullReflectionDistance,
	         uniform float        uGlobalTransparency,
	         uniform float        uNormalDistortion,
	         uniform float3       uDepthColor,
	         uniform float        uSmoothPower,
             uniform sampler2D    uNormalMap       : register(s0),
             uniform sampler2D    uReflectionMap   : register(s1),
             uniform sampler2D    uRefractionMap   : register(s2),
             uniform sampler2D    uDepthMap        : register(s3),
             uniform sampler1D    uFresnelMap      : register(s4))
{

    float2 ProjectionCoord = iUvProjection.xy / iUvProjection.w;

    float3 camToSurface = iPosition.xyz - uEyePosition;
    float additionalReflection=camToSurface.x*camToSurface.x+camToSurface.z*camToSurface.z;

    //compute the additiona reflection using  the "uFullReflectionDistance" param
    additionalReflection/=uFullReflectionDistance;

    camToSurface=normalize(-camToSurface);

    float3 pixelNormal = tex2D(uNormalMap,iUvNoise);

    //inverte y with z, because at creation our local normal to the plane was z
    pixelNormal.yz=pixelNormal.zy;
    //remap from [0,1] to [-1,1]
    pixelNormal.xyz=expand(pixelNormal.xyz);

    float2 pixelNormalModified = uNormalDistortion*pixelNormal.zx;

    float dotProduct=dot(camToSurface,pixelNormal.xyz);
    //saturate
    dotProduct=saturate(dotProduct);
    //get the fresnel term from our precomputed fresnel texture
    float fresnel = tex1D(uFresnelMap,dotProduct);

    //add additional refrection and saturate
    fresnel+=additionalReflection;
    fresnel=saturate(fresnel);

    //decrease the transparency and saturate
    fresnel-=uGlobalTransparency;
    fresnel=saturate(fresnel);

    //get the reflection pixel. make sure to disturb the texcoords by pixelnormal
    float4 reflection=tex2D(uReflectionMap,ProjectionCoord.xy+pixelNormalModified);
    //get the refraction pixel
    float4 refraction=tex2D(uRefractionMap,ProjectionCoord.xy-pixelNormalModified);

    float depth=1.0f - tex2D(uDepthMap,ProjectionCoord.xy-pixelNormalModified).r;

    refraction = lerp(refraction,float4(uDepthColor,1),depth);

    normalize(refraction);

    oColor =lerp(refraction,reflection,fresnel);

    oColor.w = saturate(depth*uSmoothPower);
}
