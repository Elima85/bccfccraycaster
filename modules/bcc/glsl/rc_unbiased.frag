#include "modules/vrn_shaderincludes.frag"

#define TEX0(p) texture(volumeStruct1_.volume_, (p - g0_off) * oneOverVoxels).a
#define TEX1(p) texture(volumeStruct2_.volume_, (p - g0_off) * oneOverVoxels).a
#define TEX2(p) texture(volumeStruct3_.volume_, (p - g0_off) * oneOverVoxels).a
#define TEX3(p) texture(volumeStruct4_.volume_, (p - g0_off) * oneOverVoxels).a

const vec3 g0_off = vec3(0.0, 0.0, 0.0);
#ifdef RECONSTRUCT_BCC
	const vec3 g1_off = vec3(0.5, 0.5, 0.5);
#endif
#ifdef RECONSTRUCT_FCC
	const vec3 g1_off = vec3(0.0, 0.5, 0.5);
	const vec3 g2_off = vec3(0.5, 0.0, 0.5);
	const vec3 g3_off = vec3(0.5, 0.5, 0.0);
#endif

vec4 result  = vec4(0.0);
vec4 result1 = vec4(0.0);
vec4 result2 = vec4(0.0);

uniform SAMPLER2D_TYPE     entryPoints_;
uniform SAMPLER2D_TYPE     entryPointsDepth_;
uniform TEXTURE_PARAMETERS entryParameters_;
uniform SAMPLER2D_TYPE     exitPoints_;
uniform SAMPLER2D_TYPE     exitPointsDepth_;
uniform TEXTURE_PARAMETERS exitParameters_;

uniform int a_;
uniform int n_;

uniform VOLUME_STRUCT volumeStruct1_;
uniform VOLUME_STRUCT volumeStruct2_;
uniform VOLUME_STRUCT volumeStruct3_;
uniform VOLUME_STRUCT volumeStruct4_;

#ifdef RECONSTRUCT_CC
	vec3 oneOverVoxels = vec3(1.0)/(volumeStruct1_.datasetDimensions_);
#else
	vec3 oneOverVoxels = vec3(1.0)/(volumeStruct1_.datasetDimensions_-.5);
#endif

const float pi = 3.141592;
const float pi2 = 6.2831853;
float adivide = 1/float(a_);

#ifdef RECONSTRUCT_BCC
	const vec3 xi[4] = vec3[](0.5 * vec3( 1,-1,-1), 0.5 * vec3(-1, 1,-1),
							  0.5 * vec3(-1,-1, 1), 0.5 * vec3( 1, 1, 1));
#endif

#ifdef RECONSTRUCT_FCC
	const vec3 xi[6] = vec3[](0.5 * vec3( 1, 1, 0), 0.5 * vec3(-1, 1, 0), 0.5 * vec3( 1, 0, 1),
							  0.5 * vec3( 1, 0,-1), 0.5 * vec3( 0, 1, 1), 0.5 * vec3( 0,-1, 1));
	
	const ivec3 bases[16] = ivec3[](ivec3(1,2,4)-1, ivec3(1,5,6)-1, ivec3(3,4,5)-1, ivec3(2,3,4)-1,
									ivec3(1,2,6)-1, ivec3(3,5,6)-1, ivec3(1,3,5)-1, ivec3(1,4,6)-1,
									ivec3(2,4,5)-1, ivec3(1,2,3)-1, ivec3(1,2,5)-1, ivec3(4,5,6)-1,
									ivec3(3,4,6)-1, ivec3(2,5,6)-1, ivec3(1,3,4)-1, ivec3(2,3,6)-1);
	
	const vec3 tau_Bcpi[16] = vec3[](pi*0.5*vec3( 1, 0, 3), pi*0.5*vec3( 3,-1, 0),
								     pi*0.5*vec3( 0,-3, 1), pi*0.5*vec3(-1,-1, 2),
								     pi*0.5*vec3( 2, 1, 1), pi*0.5*vec3( 1,-2,-1),
								     pi*0.5*vec3( 2, 0,-2), pi*0.5*vec3( 0, 2, 2),
								     pi*0.5*vec3(-2,-2, 0), pi*0.5*vec3( 1, 2,-1),
								     pi*0.5*vec3( 0, 1,-3), pi*0.5*vec3(-3, 0,-1),
								     pi*0.5*vec3(-2, 1, 1), pi*0.5*vec3(-1,-1,-2),
								     pi*0.5*vec3(-1, 3, 0), pi*0.5*vec3( 0, 0, 0));
	
#endif

vec3 sinc(vec3 x)
{
	vec3 ret = sin(x*pi)/(x*pi);
	if (abs(x).x < 0.0001)
		ret.x = 1;
	if (abs(x).y < 0.0001)
		ret.y = 1;
	if (abs(x).z < 0.0001)
		ret.z = 1;
	return ret;
}

float sinc(float x)
{
	if (abs(x) < 0.0001)
		return 1;
	return sin(x*pi)/(x*pi);
}

vec3 cc_Windowed_Sinc(vec3 x)
{	
	if (length(x) <= float(a_))
		return sinc(abs(x)) * pow(abs(sinc(x * adivide)), vec3(n_));
	else
		return vec3(0);
}

vec4 reconstructCC(in vec3 p)
{
	vec3 pv = p * volumeStruct1_.datasetDimensions_;
	
	float sum = 0;
	vec3 offset = fract(pv);
	vec3 v;
	
	for (v.x = -a_; v.x <= a_; v.x++)
	{
		for (v.y = -a_; v.y <= a_; v.y++)
		{
			for (v.z = -a_; v.z <= a_; v.z++)
			{
				float sample0 = TEX0(pv + v);
				vec3 L = cc_Windowed_Sinc(offset - v);
				sum += sample0 * L.x * L.y * L.z;
			}
		}
	}
	return vec4(0, 0, 0, sum);
}

#ifdef RECONSTRUCT_BCC
float BCC_Sinc_L(vec3 x)
{
	float ret = 0;	
	for (int i = 0; i < 4; i++)
	{
		float temp = 1;
		for (int j = 0; j < 4; j++)
			if (i != j)
				temp *= sinc(xi[j].x*x.x + xi[j].y*x.y + xi[j].z*x.z);
				
		ret += temp * cos(pi*(xi[i].x*x.x + xi[i].y*x.y + xi[i].z*x.z));
	}
	return ret * 0.25;
}

float BCC_Windowed_Sinc_L(vec3 x)
{
	if (abs(x.x) < a_)
		if (abs(x.y) < a_)
			if (abs(x.z) < a_)
		return BCC_Sinc_L(x) * pow(abs(BCC_Sinc_L(x*adivide)), float(n_));
	return 0;
}

vec4 reconstructBCC(in vec3 p)
{
	vec3 pv = p * volumeStruct1_.datasetDimensions_;
	vec3 pr = floor(pv);

	vec3 offset0 = pv - pr;
	vec3 offset1 = offset0 - g1_off;
	
	float side = float(a_) + 1;
	float sum = 0;
	vec3 v;
	for (v.x = -side; v.x <= side; v.x++)
	{
		for (v.y = -side; v.y <= side; v.y++)
		{
			for (v.z = -side; v.z <= side; v.z++)
			{
				float window0 = BCC_Windowed_Sinc_L(offset0 - v);
				float window1 = BCC_Windowed_Sinc_L(offset1 - v);
				sum += window0 * TEX0(pr + v);
				sum += window1 * TEX1(pr + v);
			}
		}
	}
	return vec4(0, 0, 0, sum);
}
#endif

#ifdef RECONSTRUCT_FCC
float FCC_Sinc_L(vec3 x)
{
	float ret = 0;
	for (int i = 0; i < 16; i++)
		ret += cos(tau_Bcpi[i].x*x.x + tau_Bcpi[i].y*x.y + tau_Bcpi[i].z*x.z)
			* sinc(xi[bases[i].x].x*x.x + xi[bases[i].x].y*x.y + xi[bases[i].x].z*x.z)
			* sinc(xi[bases[i].y].x*x.x + xi[bases[i].y].y*x.y + xi[bases[i].y].z*x.z)
			* sinc(xi[bases[i].z].x*x.x + xi[bases[i].z].y*x.y + xi[bases[i].z].z*x.z);
	return ret * 0.0625;
}

float FCC_Windowed_Sinc_L(vec3 x)
{
	if (abs(x.x) < a_)
		if (abs(x.y) < a_)
			if (abs(x.z) < a_)
				return FCC_Sinc_L(x) * pow(abs(FCC_Sinc_L(x*adivide)), float(n_));
	return 0;
}

vec4 reconstructFCC(in vec3 p)
{
	vec3 pv = p * volumeStruct1_.datasetDimensions_;
	vec3 pr = floor(pv);
	
	vec3 offset0 = pv - pr;
	vec3 offset1 = offset0 - g1_off;
	vec3 offset2 = offset0 - g2_off;
	vec3 offset3 = offset0 - g3_off;
	
	float side = float(a_) + 1;
	float sum = 0;
	vec3 v;	
	for (v.x = -side; v.x <= side; v.x++)
	{
		for (v.y = -side; v.y <= side; v.y++)
		{
			for (v.z = -side; v.z <= side; v.z++)
			{
				float window0 = FCC_Windowed_Sinc_L(offset0 - v);
				float window1 = FCC_Windowed_Sinc_L(offset1 - v);
				float window2 = FCC_Windowed_Sinc_L(offset2 - v);
				float window3 = FCC_Windowed_Sinc_L(offset3 - v);
				sum += window0 * TEX0(pr + v);
				sum += window1 * TEX1(pr + v);
				sum += window2 * TEX2(pr + v);
				sum += window3 * TEX3(pr + v);
			}
		}
	}
	return vec4(0, 0, 0, sum);
}
#endif

void rayTraversal(in vec3 first, in vec3 last)
{
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    vec3 rayDirection;
    raySetup(first, last, volumeStruct1_.datasetDimensions_, rayDirection, tIncr, tEnd);
	
	
	#ifdef reconstructBCC
		tIncr /= 1.259921049894873;
	#endif
	
	#ifdef reconstructFCC
		tIncr /= 1.587401051968200;
	#endif

    RC_BEGIN_LOOP {
        vec3 samplePos = first + t * rayDirection;
        vec4 voxel = RC_APPLY_RECONSTRUCTION(samplePos);
		
        vec4 color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel);

        color.rgb = RC_APPLY_SHADING(voxel.xyz, samplePos, volumeStruct1_, color.rgb, color.rgb, vec3(1.0,1.0,1.0));

        if (color.a > 0.0) {
            RC_BEGIN_COMPOSITING
            result = RC_APPLY_COMPOSITING(result, color, samplePos, voxel.xyz, t, tDepth)
            result1 = RC_APPLY_COMPOSITING_1(result1, color, samplePos, voxel.xyz, t, tDepth)
            result2 = RC_APPLY_COMPOSITING_2(result2, color, samplePos, voxel.xyz, t, tDepth)
            RC_END_COMPOSITING
        }
    } RC_END_LOOP(result);
}

void main()
{
    vec3 frontPos = textureLookup2D(entryPoints_, entryParameters_, gl_FragCoord.xy).rgb;
    vec3 backPos = textureLookup2D(exitPoints_, exitParameters_, gl_FragCoord.xy).rgb;

    if (frontPos == backPos)
        discard;
	else
        rayTraversal(frontPos, backPos);

	#ifdef OP0
		FragData0 = result;
	#endif
	#ifdef OP1
		FragData1 = result1;
	#endif
	#ifdef OP2
		FragData2 = result2;
	#endif
}