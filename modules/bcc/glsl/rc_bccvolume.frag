#include "modules/vrn_shaderincludes.frag"

#define NORM(v) ((v) / (volumeStruct1_.datasetDimensions_))

#ifndef VOLUME_FORMAT_INTERLEAVED
	#define SAMPLE vec4
	#define TEX0(p) textureLookup3D(volumeStruct1_, NORM((p) - g0_off))
	#define TEX1(p) textureLookup3D(volumeStruct2_, NORM((p) - g1_off))
#else
	#define SAMPLE float
	#define TEX0(p) textureLookup3D(volumeStruct1_, NORM((p) - g0_off)).r
	#define TEX1(p) textureLookup3D(volumeStruct1_, NORM((p) - g1_off)).g
#endif

#ifdef Z_INTERLEAVED
	#define TEX(p) textureLookup3D(volumeStruct1_, floor(p*convert)*oneOverVoxels)
#endif

const vec3 g0_off = vec3(0.0, 0.0, 0.0);
const vec3 g1_off = vec3(0.5, 0.5, 0.5);

vec4 result  = vec4(0.0);
vec4 result1 = vec4(0.0);
vec4 result2 = vec4(0.0);

uniform SAMPLER2D_TYPE     entryPoints_;
uniform SAMPLER2D_TYPE     entryPointsDepth_;
uniform TEXTURE_PARAMETERS entryParameters_;
uniform SAMPLER2D_TYPE     exitPoints_;
uniform SAMPLER2D_TYPE     exitPointsDepth_;
uniform TEXTURE_PARAMETERS exitParameters_;

uniform float lambda_; //used for CWB reconstruction

uniform VOLUME_STRUCT volumeStruct1_;
#ifndef VOLUME_FORMAT_INTERLEAVED
	uniform VOLUME_STRUCT volumeStruct2_;
#endif

#ifdef Z_INTERLEAVED
	vec3 oneOverVoxels = vec3(1.0)/(volumeStruct1_.datasetDimensions_ - 0.5);
	vec3 convert = vec3(0.5, 0.5, 1.0);
#endif

vec4 reconstructDC(in vec3 p)
{
	vec3 pw = p * volumeStruct1_.datasetDimensions_ - 0.5;

	vec3 v0 = round(pw - g0_off) + g0_off;
	vec3 v1 = round(pw - g1_off) + g1_off;

	mat3 flipMatrix =
		mat3(vec3(sign(v1.x - v0.x), 0.0, 0.0),
	         vec3(0.0, sign(v1.y - v0.y), 0.0),
	         vec3(0.0, 0.0, sign(v1.z - v0.z)));

	/* Interpolate unknown points that we need. */
	SAMPLE p100 = TEX0(v0 + flipMatrix * vec3( 0.5,  0.0,  0.0) + 0.5);
	SAMPLE p110 = TEX1(v1 + flipMatrix * vec3( 0.0,  0.0, -0.5) + 0.5);
	SAMPLE p001 = TEX0(v0 + flipMatrix * vec3( 0.0,  0.0,  0.5) + 0.5);
	SAMPLE p011 = TEX1(v1 + flipMatrix * vec3(-0.5,  0.0,  0.0) + 0.5);

	/* Interpolate quad corners. */
	SAMPLE q00 = TEX0(vec3(v0.x, pw.y, v0.z) + 0.5);
	SAMPLE q11 = TEX1(vec3(v1.x, pw.y, v1.z) + 0.5);
	SAMPLE q01 = mix(p001, p011, 2.0 * abs(pw.y - v0.y));
	SAMPLE q10 = mix(p110, p100, 2.0 * abs(pw.y - v1.y));

	/* Bilinearly interpolate quad. */
    SAMPLE left  = mix(q00, q01, 2.0 * abs(pw.z - v0.z));
    SAMPLE right = mix(q11, q10, 2.0 * abs(pw.z - v1.z));

	SAMPLE value = mix(left, right, 2.0 * abs(pw.x - v0.x));

	#ifndef VOLUME_FORMAT_INTERLEAVED
		value.rgb = (value.rgb - vec3(0.5)) * 2.0;
	    return value;
	#else
		return vec4(0.0, 0.0, 0.0, value);
	#endif
}

vec4 reconstructLinbox(vec3 p)
{
	vec3 P1, P2, P3, P4;
	SAMPLE D1, D2, D3, D4;
	
	#ifndef Z_INTERLEAVED
		vec3 posOS = p * volumeStruct1_.datasetDimensions_ * 2.0;
	#else
		vec3 posOS = p * vec3(volumeStruct1_.datasetDimensions_.xy * 2.0,
			volumeStruct1_.datasetDimensions_.z) - 1;
	#endif
	
	vec3 abc = vec3(posOS.x + posOS.y,
	                posOS.x + posOS.z,
	                posOS.y + posOS.z) * 0.5;

	vec3 floors = floor(abc);
	abc = abc - floors;

	P1 = vec3( floors.x + floors.y - floors.z,
	           floors.x - floors.y + floors.z,
	          -floors.x + floors.y + floors.z);

	P2 = P1 + vec3(1.0, 1.0,  1.0);
	P3 = P1 + vec3(1.0, 1.0, -1.0);
	P4 = P1 + vec3(2.0, 0.0,  0.0);

	vec4 sorting = vec4(1.0, 0.0, 0.0, 0.0);

	sorting.y = max(abc.x, max(abc.y, abc.z));
	sorting.z = min(abc.x, min(abc.y, abc.z));
	sorting.w = (abc.x + abc.y + abc.z) - sorting.y - sorting.z;

	P3 += (float(sorting.y == abc.y) * vec3( 0.0, -2.0, 2.0) +
	       float(sorting.y == abc.z) * vec3(-2.0,  0.0, 2.0));

	P4 += (float(sorting.z == abc.x) * vec3(-2.0,  0.0, 2.0) +
	       float(sorting.z == abc.y) * vec3(-2.0,  2.0, 0.0));

	#ifndef Z_INTERLEAVED
		D1 = (bool(int(P1.x) % 2) ? TEX0(P1 / 2.0) : TEX1(P1 / 2.0));
		D2 = (bool(int(P2.x) % 2) ? TEX0(P2 / 2.0) : TEX1(P2 / 2.0));
		D3 = (bool(int(P3.x) % 2) ? TEX0(P3 / 2.0) : TEX1(P3 / 2.0));
		D4 = (bool(int(P4.x) % 2) ? TEX0(P4 / 2.0) : TEX1(P4 / 2.0));
	#else
		D1 = TEX(P1);
		D2 = TEX(P2);
		D3 = TEX(P3);
		D4 = TEX(P4);
	#endif

	#ifndef VOLUME_FORMAT_INTERLEAVED
		vec4 value = vec4(dot(sorting, vec4(D1[0], D3[0] - D1[0], D2[0] - D4[0], D4[0] - D3[0])),
			              dot(sorting, vec4(D1[1], D3[1] - D1[1], D2[1] - D4[1], D4[1] - D3[1])),
						  dot(sorting, vec4(D1[2], D3[2] - D1[2], D2[2] - D4[2], D4[2] - D3[2])),
			              dot(sorting, vec4(D1[3], D3[3] - D1[3], D2[3] - D4[3], D4[3] - D3[3])));
		value.rgb = (value.rgb - 0.5) * 2.0;
		return value;
	#else
		vec4 values = vec4(D1, D3 - D1, D2 - D4, D4 - D3);
		return vec4(0.0, 0.0, 0.0, dot(sorting, values));
	#endif
}

vec4 reconstructNearest(in vec3 p)
{
	#ifndef Z_INTERLEAVED
		const vec3 offsets[2] = vec3[2](g0_off, g1_off);

		vec3 pw = p * volumeStruct1_.datasetDimensions_ - 0.5;

		vec3 v0 = round(pw + g0_off) - g0_off;
		vec3 v1 = round(pw + g1_off) - g1_off;

		SAMPLE values[2] = SAMPLE[2](TEX1(v1 + 0.5), TEX0(v0 + 0.5));
		SAMPLE value = values[int(distance(pw, v0) < distance(pw, v1))];

		value *= volumeStruct1_.bitDepthScale_;
		
		#ifndef VOLUME_FORMAT_INTERLEAVED
			value.a *= volumeStruct1_.rwmScale_;
			value.a += volumeStruct1_.rwmOffset_;

			value.rgb = (value.rgb - vec3(0.5)) * 2.0;
			return value;
		#else
			value *= volumeStruct1_.rwmScale_;
			value += volumeStruct1_.rwmOffset_;

			return vec4(0.0, 0.0, 0.0, value);
		#endif
	#else
		vec3 pw = p * vec3(2*volumeStruct1_.datasetDimensions_.xy-1.0,
							volumeStruct1_.datasetDimensions_.z-1.0);
		SAMPLE value = TEX(pw);
		#ifndef VOLUME_FORMAT_INTERLEAVED
			value.rgb = (value.rgb - 0.5) * 2.0;
			return value;
		#else
			return vec4(0.0, 0.0, 0.0, value);
			#endif
	#endif
}

vec4 reconstructCWB(in vec3 p)
{
	const float size = volumeStruct1_.datasetDimensions_;
	const float pi2 = 6.283185;
	
	vec3 pw = p*size;
	
	SAMPLE sample0 = TEX0(pw);
	SAMPLE sample1 = TEX1(pw);
	
	vec3 p0 = NORM(pw)*size;
	vec3 c = cos(p0*pi2);
	float w = 0.5 + (c.x + c.y + c.z) / 6.0 * lambda_;
	
	SAMPLE value = mix(sample0, sample1, w);
	
	#ifndef VOLUME_FORMAT_INTERLEAVED
		value.rgb = (value.rgb - vec3(0.5)) * 2.0;
		return value;
	#else
		return vec4(0.0, 0.0, 0.0, value);
	#endif
}

void rayTraversal(in vec3 first, in vec3 last)
{
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    vec3 rayDirection;
	vec3 size = volumeStruct1_.datasetDimensions_;
	
	#ifdef Z_INTERLEAVED
		size.z *= 0.5;
	#endif
	
    raySetup(first, last, size, rayDirection, tIncr, tEnd);
	
    tIncr /= 1.259921049894873;
	
    RC_BEGIN_LOOP {
		vec3 samplePos = first + t * rayDirection;
		
        vec4 voxel = RC_APPLY_RECONSTRUCTION(samplePos);
		
        vec4 color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel);

		#ifdef Z_INTERLEAVED
			samplePos.z *= 0.5;
		#endif
		
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
    vec3 frontPos = textureLookup2D(entryPoints_, entryParameters_, gl_FragCoord.xy).xyz;
    vec3 backPos = textureLookup2D(exitPoints_, exitParameters_, gl_FragCoord.xy).xyz;
	
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

