#include "modules/vrn_shaderincludes.frag"

#ifndef NO_SHADING
	#define SAMPLE vec4
#else
	#define SAMPLE float
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

uniform VOLUME_STRUCT volumeStruct_;

vec3 convert = vec3(0.5, 0.5, 1.0); //conversion between 3D coords and texture index
vec3 oneOverVoxels = vec3(1.0)/vec3(volumeStruct_.datasetDimensions_-1.0);
vec3 size_volume = vec3(2*volumeStruct_.datasetDimensions_.x-1.0, 2*volumeStruct_.datasetDimensions_.y-1.0,
	volumeStruct_.datasetDimensions_.z-1.0);

vec4 reconstructLinbox(vec3 p)
{
	vec3 P1, P2, P3, P4;
	vec4 D1, D2, D3, D4;
	
	vec3 posOS = (p * size_volume);

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

	P3 += (int(sorting.y == abc.y) * vec3( 0.0, -2.0, 2.0) +
	       int(sorting.y == abc.z) * vec3(-2.0,  0.0, 2.0));

	P4 += (int(sorting.z == abc.x) * vec3(-2.0,  0.0, 2.0) +
	       int(sorting.z == abc.y) * vec3(-2.0,  2.0, 0.0));
	
	D1 = textureLookup3D(volumeStruct_, (floor(P1*convert)) * oneOverVoxels);
	D2 = textureLookup3D(volumeStruct_, (floor(P2*convert)) * oneOverVoxels);
	D3 = textureLookup3D(volumeStruct_, (floor(P3*convert)) * oneOverVoxels);
	D4 = textureLookup3D(volumeStruct_, (floor(P4*convert)) * oneOverVoxels);
	
	#ifndef NO_SHADING
		vec4 value = vec4(dot(sorting, vec4(D1[0], D3[0] - D1[0], D2[0] - D4[0], D4[0] - D3[0])),
			              dot(sorting, vec4(D1[1], D3[1] - D1[1], D2[1] - D4[1], D4[1] - D3[1])),
						  dot(sorting, vec4(D1[2], D3[2] - D1[2], D2[2] - D4[2], D4[2] - D3[2])),
			              dot(sorting, vec4(D1[3], D3[3] - D1[3], D2[3] - D4[3], D4[3] - D3[3])));
		value.xyz = (value.xyz - 0.5) * 2.0;
		return value;
	#else
		vec4 values = vec4(D1.a, D3.a - D1.a, D2.a - D4.a, D4.a - D3.a);
		return vec4(0.0, 0.0, 0.0, dot(sorting, values));
	#endif
}

vec4 reconstructNearest(in vec3 p)
{
	vec3 pw = p * (size_volume);
	
	float value = textureLookup3D(volumeStruct_, (floor(pw*convert)) * oneOverVoxels).a;
	
	return vec4(0.0, 0.0, 0.0, value);
}

void rayTraversal(in vec3 first, in vec3 last)
{
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    vec3 rayDirection;
    raySetup(first, last, volumeStruct_.datasetDimensions_, rayDirection, tIncr, tEnd);
    tIncr /= 1.259921049894873;
	
    RC_BEGIN_LOOP {
        vec3 samplePos = first + t * rayDirection;
        vec4 voxel = RC_APPLY_RECONSTRUCTION(samplePos);
		
        vec4 color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel);
		
		samplePos.z *= .5; //needed to make shading work correctly
		
		color.rgb = RC_APPLY_SHADING(voxel.xyz, samplePos, volumeStruct_, color.rgb, color.rgb, vec3(1.0,1.0,1.0));
		
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

