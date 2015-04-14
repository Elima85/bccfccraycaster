#include "modules/vrn_shaderincludes.frag"

#define NORM(v) ((v) / volumeStruct1_.datasetDimensions_)

#ifndef VOLUME_FORMAT_INTERLEAVED
	#define SAMPLE vec4
	#define TEX0(p) texture(volumeStruct1_.volume_, NORM((p) - g0_off))
	#define TEX1(p) texture(volumeStruct2_.volume_, NORM((p) - g1_off))
	#define TEX2(p) texture(volumeStruct3_.volume_, NORM((p) - g2_off))
	#define TEX3(p) texture(volumeStruct4_.volume_, NORM((p) - g3_off))
#else
	#define SAMPLE float
	#define TEX0(p) texture(volumeStruct1_.volume_, NORM((p) - g0_off)).r
	#define TEX1(p) texture(volumeStruct1_.volume_, NORM((p) - g1_off)).g
	#define TEX2(p) texture(volumeStruct1_.volume_, NORM((p) - g2_off)).b
	#define TEX3(p) texture(volumeStruct1_.volume_, NORM((p) - g3_off)).a
#endif

const vec3 g0_off = vec3(0.0, 0.0, 0.0);
const vec3 g1_off = vec3(0.0, 0.5, 0.5);
const vec3 g2_off = vec3(0.5, 0.0, 0.5);
const vec3 g3_off = vec3(0.5, 0.5, 0.0);

vec4 result  = vec4(0.0);
vec4 result1 = vec4(0.0);
vec4 result2 = vec4(0.0);

uniform SAMPLER2D_TYPE     entryPoints_;
uniform SAMPLER2D_TYPE     entryPointsDepth_;
uniform TEXTURE_PARAMETERS entryParameters_;
uniform SAMPLER2D_TYPE     exitPoints_;
uniform SAMPLER2D_TYPE     exitPointsDepth_;
uniform TEXTURE_PARAMETERS exitParameters_;

uniform VOLUME_STRUCT volumeStruct1_;
#ifndef VOLUME_FORMAT_INTERLEAVED
	uniform VOLUME_STRUCT volumeStruct2_;
	uniform VOLUME_STRUCT volumeStruct3_;
	uniform VOLUME_STRUCT volumeStruct4_;
#endif

vec4 reconstructDC(in vec3 p)
{
	vec3 pw = p * volumeStruct1_.datasetDimensions_ - 0.5;

	/* Find corners. */
	vec3 v0 = round(pw);                   /* White. */
	vec3 v1 = round(pw + g1_off) - g1_off; /* White-black. */
	vec3 v2 = round(pw + g2_off) - g2_off; /* Black. */
	vec3 v3 = round(pw + g3_off) - g3_off; /* White-white. */

	/* Make a matrix to flip offsets based on corner orientation. */
	mat3 flipMatrix =
		mat3(vec3(sign(v2.x - v0.x), 0.0, 0.0),
	         vec3(0.0, sign(v3.y - v0.y), 0.0),
	         vec3(0.0, 0.0, sign(v1.z - v0.z)));

	/* Interpolate unknown points that we need. */
	vec3 samp;

	samp = v0 + flipMatrix * vec3(0.5, 0.0, 0.0) + 0.5;
	SAMPLE p100 = (TEX0(samp) + 
	               TEX3(samp) +
	               TEX2(samp)) / 3.0;

	samp = v0 + flipMatrix * vec3(0.0, 0.5, 0.0) + 0.5;
	SAMPLE p010 = (TEX0(samp) +
	               TEX3(samp) +
	               TEX1(samp)) / 3.0;

	samp = v0 + flipMatrix * vec3(0.0, 0.0, 0.5) + 0.5;
	SAMPLE p001 = (TEX0(samp) +
	               TEX2(samp) + 
	               TEX1(samp)) / 3.0;

	samp = v0 + flipMatrix * vec3(0.5, 0.5, 0.5) + 0.5;
	SAMPLE p111 = (TEX2(samp) +
	               TEX1(samp) +
	               TEX3(samp)) / 3.0;

	SAMPLE p000 = TEX0(v0 + 0.5);
	SAMPLE p011 = TEX1(v1 + 0.5);
	SAMPLE p101 = TEX2(v2 + 0.5);
	SAMPLE p110 = TEX3(v3 + 0.5);

	/* Interpolate quad corners. */
	SAMPLE q00 = mix(p000, p010, 2.0 * abs(pw.y - v0.y));
	SAMPLE q11 = mix(p101, p111, 2.0 * abs(pw.y - v0.y));
	SAMPLE q01 = mix(p001, p011, 2.0 * abs(pw.y - v0.y));
	SAMPLE q10 = mix(p100, p110, 2.0 * abs(pw.y - v0.y));

	/* Bilinearly interpolate quad. */
    SAMPLE left  = mix(q00, q01, 2.0 * abs(pw.z - v0.z));
    SAMPLE right = mix(q11, q10, 2.0 * abs(pw.z - v1.z));
    SAMPLE value = mix(left, right, 2.0 * abs(pw.x - v0.x));

	#ifndef VOLUME_FORMAT_INTERLEAVED
		value.xyz -= 0.5;
		return value;
	#else
		return vec4(0.0, 0.0, 0.0, value);
	#endif
}

vec4 reconstructNearest(in vec3 X)
{
	vec3 x = X * volumeStruct1_.datasetDimensions_ - 0.5;

	vec3 j[4];
	j[0] = round(x);
	j[1] = round(x + g1_off) - g1_off;
	j[2] = round(x + g2_off) - g2_off;
	j[3] = round(x + g3_off) - g3_off;

	float jd[4];
	jd[0] = distance(j[0], x);
	jd[1] = distance(j[1], x);
	jd[2] = distance(j[2], x);
	jd[3] = distance(j[3], x);

	SAMPLE value;

	if (jd[0] < jd[1]) {
		if (jd[0] < jd[2]) {
			if (jd[0] < jd[3])
				value = TEX0(j[0] + 0.5);
			else
				value = TEX3(j[3] + 0.5);
		}
		else {
			if (jd[2] < jd[3])
				value = TEX2(j[2] + 0.5);
			else
				value = TEX3(j[3] + 0.5);
		}
	}
	else {
		if (jd[1] < jd[2]) {
			if (jd[1] < jd[3])
				value = TEX1(j[1] + 0.5);
			else
				value = TEX3(j[3] + 0.5);
		}
		else {
			if (jd[2] < jd[3])
				value = TEX2(j[2] + 0.5);
			else
				value = TEX3(j[3] + 0.5);
		}
	}

	#ifndef VOLUME_FORMAT_INTERLEAVED
		value.xyz = (value.xyz - vec3(0.5)) * 2.0;
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
    raySetup(first, last, volumeStruct1_.datasetDimensions_, rayDirection, tIncr, tEnd);
	
	//adjust sampling rate for FCC by 4^(1/3) = 1.5874...
	tIncr /= 1.587401051968200;

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

