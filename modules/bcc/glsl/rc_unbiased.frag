#include "modules/vrn_shaderincludes.frag"

#define TEX0(p) textureLookup3D(volumeStruct1_, (p - g0_off) * oneOverVoxels).a
#define TEX1(p) textureLookup3D(volumeStruct2_, (p - g1_off) * oneOverVoxels).a
#define TEX2(p) textureLookup3D(volumeStruct3_, (p - g2_off) * oneOverVoxels).a
#define TEX3(p) textureLookup3D(volumeStruct4_, (p - g3_off) * oneOverVoxels).a

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

const vec3 oneOverVoxels = vec3(1.0)/(volumeStruct1_.datasetDimensions_);
const float pi = 3.141592;
const float pi2 = 6.2831853;
const float adivide = 1/float(a_);

#ifdef RECONSTRUCT_BCC
	/*const mat3 L = mat3(-1,  1,  1,
						 1, -1,  1,
						 1,  1, -1);*/
						 
	const float absDetL = 4; //abs(determinant(L)); //L = 4
	
	const vec3 xi_1 = 0.25 * vec3( 1, -1, -1);
	const vec3 xi_2 = 0.25 * vec3(-1,  1, -1);
	const vec3 xi_3 = 0.25 * vec3(-1, -1,  1);
	const vec3 xi_4 = 0.25 * vec3( 1,  1,  1);
	
	/*const mat3 B[4] = mat3[](	mat3(xi[0], xi[1], xi[2]), 
								mat3(xi[0], xi[1], xi[3]),
								mat3(xi[0], xi[2], xi[3]),
								mat3(xi[1], xi[2], xi[3]));*/
	
	const float det_B_1 = 0.0625; // abs(determinant(B[0])); // 0.0625
	const float det_B_2 = 0.0625; // abs(determinant(B[1])); // 0.0625
	const float det_B_3 = 0.0625; // abs(determinant(B[2])); // 0.0625
	const float det_B_4 = 0.0625; // abs(determinant(B[3])); // 0.0625
	
	const vec3 tau_B_1 = -0.125 * vec3( 1,  1,  1);
	const vec3 tau_B_2 = -0.125 * vec3(-1, -1,  1);
	const vec3 tau_B_3 = -0.125 * vec3(-1,  1, -1);
	const vec3 tau_B_4 = -0.125 * vec3( 1, -1, -1);
#endif

#ifdef RECONSTRUCT_FCC
	const mat3 L = mat3( 0,  1,  1,
						 1,  0,  1,
						 1,  1,  0);
	const float detL = determinant(L);
						 
	const vec3 xi_1 = 0.25 * vec3( 1,  1,  0);
	const vec3 xi_2 = 0.25 * vec3(-1,  1,  0);
	const vec3 xi_3 = 0.25 * vec3( 1,  0,  1);
	const vec3 xi_4 = 0.25 * vec3( 1,  0, -1);
	const vec3 xi_5 = 0.25 * vec3( 0,  1,  1);
	const vec3 xi_6 = 0.25 * vec3( 0, -1,  1);
#endif

vec3 sinc(vec3 x)
{
	if (x == vec3(0))
		return vec3(1);
	return sin(x*pi)/(x*pi);
}

float sinc(float x)
{
	if (x == 0)
		return 1;
	return sin(x*pi)/(x*pi);
}

vec3 cc_Windowed_Sinc_L(vec3 x)
{
	return (1 - step(a_, abs(x.x))) * (1 - step(a_, abs(x.y))) * (1 - step(a_, abs(x.z))) * sinc(x)
		* pow(abs(sinc(x * adivide)), vec3(n_));
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
				vec3 L = cc_Windowed_Sinc_L(offset - v);
				sum += sample0 * L.x * L.y * L.z;
			}
		}
	}
	return vec4(0, 0, 0, sum);
}

#ifdef RECONSTRUCT_BCC
float bccSinc_L(vec3 x)
{
	float ret = 0;

	ret += det_B_1 * cos(pi2 * dot(tau_B_1, x)) 
		* sinc(dot(xi_1, x))
		* sinc(dot(xi_2, x))
		* sinc(dot(xi_3, x));
		
	ret += det_B_2 * cos(pi2 * dot(tau_B_2, x)) 
		* sinc(dot(xi_1, x))
		* sinc(dot(xi_2, x))
		* sinc(dot(xi_4, x));
	
	ret += det_B_3 * cos(pi2 * dot(tau_B_3, x)) 
		* sinc(dot(xi_1, x))
		* sinc(dot(xi_3, x))
		* sinc(dot(xi_4, x));
	
	ret += det_B_4 * cos(pi2 * dot(tau_B_4, x)) 
		* sinc(dot(xi_2, x))
		* sinc(dot(xi_3, x))
		* sinc(dot(xi_4, x));
		
	ret *= absDetL;
	
	return ret;
}

float bcc_Windowed_Sinc_L(vec3 x, float aValue)
{
	return (1 - step(aValue, abs(x.x))) * (1 - step(aValue, abs(x.y))) * (1 - step(aValue, abs(x.z))) * bccSinc_L(x)
		* pow(abs(bccSinc_L(x / aValue)), float(n_));
}


vec4 reconstructBCC(in vec3 p)
{	
	vec3 pv = p * volumeStruct1_.datasetDimensions_;
	
	float sum = 0;
	vec3 offset = fract(pv);
	vec3 v;
	float aVal = float(a_) * 0.79;
	float kernelSize = .5;
	float stepLength = 1;
	for (v.x = -kernelSize; v.x <= kernelSize; v.x += stepLength)
	{
		for (v.y = -kernelSize; v.y <= kernelSize; v.y += stepLength)
		{
			for (v.z = -kernelSize; v.z <= kernelSize; v.z += stepLength)
			{
				vec3 pw = pv + v;
				vec3 v0 = round(pw + g0_off) - g0_off;
				vec3 v1 = round(pw + g1_off) - g1_off;
				int index = int(distance(pw, v0) < distance(pw, v1));
				float values[2] = float[2](TEX1(v1 + 0.5), TEX0(v0 + 0.5));				
				float L[2] = float [2](
					bcc_Windowed_Sinc_L(offset - v - g1_off, aVal),
					bcc_Windowed_Sinc_L(offset - v, aVal));
				sum += values[index] * L[index];
				
				/*float sample0 = TEX0(pv + v);
				float L = bcc_Windowed_Sinc_L(offset - v, aVal);
				sum += sample0 * L;*/
			}
		}
	}
	return vec4(0, 0, 0, sum);
}
#endif

vec4 reconstructFCC(in vec3 p)
{
	// not implemented yet
	return vec4(0.8 * float(a_) * float(n_));
}

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

