#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform UniformBufferObject
{
    vec2 resolution;
} ubo;

#define iResolution ubo.resolution

#define CLOSE_ENOUGH .001
#define BLACK vec3(.1)
#define WHITE vec3(1.0,.95,.95)
#define YELLOW vec3(1.0,1.0,.0)

struct sdf_return
{
	float sdf;
    vec3 col;
};

struct sphere
{
	vec3 c;
    float r;
};
    
struct ray
{
	vec3 o;
    vec3 d;
};
    
struct dirLight
{
	vec3 d;
    float i;
};
    
const int sphereCount = 11;
    
float sphDist(vec3 p, sphere s)
{
	return distance(p, s.c) - s.r;
}

//from iq : https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float dist_smooth( float d1, float d2, float k ) 
{
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h); 
}

float sdf(vec3 p, sphere[sphereCount] spheres)
{
    float sdist1 = sphDist(p, spheres[ 0]);
    float sdist2 = sphDist(p, spheres[ 1]);
    float sdist3 = sphDist(p, spheres[ 2]);
    float sdist4 = sphDist(p, spheres[ 3]);
    float sdist5 = sphDist(p, spheres[ 4]);
    float sdist6 = sphDist(p, spheres[ 5]);
    float sdist7 = sphDist(p, spheres[ 6]);
    float sdist8 = sphDist(p, spheres[ 7]);
    float sdist9 = sphDist(p, spheres[ 8]);
    float sdist11= sphDist(p, spheres[10]);
    
    float body = dist_smooth(sdist1, sdist2, .1);
    float leg1 = dist_smooth(dist_smooth(sdist3,sdist4,.15), sdist5, .1);
    float leg2 = dist_smooth(dist_smooth(sdist6,sdist7,.15), sdist8, .1);
	
    
    
    return min(min(min(dist_smooth(body, sdist9,.1), leg1),leg2),sdist11);
}

sdf_return scene(vec3 p, sphere[sphereCount] spheres)
{
    vec3 col = BLACK;
    float body = sdf(p, spheres);
    float eye = sphDist(p, spheres[9]);
    if(eye < body) col = WHITE;
    
    return sdf_return(min(eye, body), col);
}

//from iq https://www.iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec3 calcNormal( in vec3 p, sphere[sphereCount] spheres )
{
    const vec2 k = vec2(1,-1);
    return normalize( k.xyy*sdf( p + k.xyy*CLOSE_ENOUGH, spheres ) + 
                      k.yyx*sdf( p + k.yyx*CLOSE_ENOUGH, spheres ) + 
                      k.yxy*sdf( p + k.yxy*CLOSE_ENOUGH, spheres ) + 
                      k.xxx*sdf( p + k.xxx*CLOSE_ENOUGH, spheres ) );
}

const int aa = 2;

void main()
{
    
    
    sphere spheres[sphereCount] = sphere[sphereCount]
	(
		sphere(vec3(.1,.2, .5)						,.2		),	//BODY
	    sphere(vec3(.0,.0,.5)						,.1		),	//BODY
        sphere(vec3(.1,-.01, .45)					,.02	),	//LEG1
        sphere(vec3(.12,-.1, .45)					,.017	),	//LEG1
        sphere(vec3(.13,-.18, .45)					,.014	),	//LEG1
        sphere(vec3(-.01,-.01, .45)					,.02	),	//LEG2
        sphere(vec3(-.012,-.1, .45)					,.017	),	//LEG2
        sphere(vec3(-.013,-.18, .45)				,.014	),	//LEG2
        sphere(vec3(-.13+sin(.0)*.01, -.04, .45)	,.06	), 	//HEAD
        sphere(vec3(-.13+sin(.0)*.01, -.04, .1)	    ,.015	), 	//EYE_WHITE
        sphere(vec3(-.13+sin(.0)*.01, -.04, .09)	,.01	) 	//EYE_BLACK
	);
   	
    dirLight l = dirLight(normalize(vec3(1.0, 1.0, -.2)), 1.0);
	
    vec3 total = vec3(.0);
    for(int i = 0; i < aa; i++)
    for(int j = 0; j < aa; j++)
    {
    	vec2 uv = (gl_FragCoord.xy+vec2(float(i),float(j))/float(aa)-.5)/iResolution.xy - vec2(.5);
        uv.x *= iResolution.x/iResolution.y;
        
    
    
    	ray r = ray(vec3(uv.x, uv.y, .0), normalize(vec3(uv.xy, 4.0)));
    	
    	vec3 col = YELLOW;
    	
    	float dist = .0;
    	while(dist < 50.0)
    	{
    	    vec3 rp = r.o + r.d *dist;
    	    sdf_return ret = scene(rp, spheres);
    	    if(ret.sdf < CLOSE_ENOUGH)
    	    {
    	    	vec3 n = calcNormal(rp,spheres);
    	        col = ret.col;
    	        if(col.r >.2) col *=2.0;
				col *= (.15 + max(dot(l.d, n), .0)*.45 + pow(clamp(dot(reflect(r.d, n), r.d), .0, 1.0), 32.0)*.4) * l.i; 
    	        break;
    	    }
    	    dist += ret.sdf;
    	}
        total += col;
    }
    total /= float(aa*aa);
    
    outColor = vec4(pow(total, vec3(1./2.2)),1.0);
}