#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform UniformBufferObject
{
    vec2 resolution;
} ubo;

#define iResolution ubo.resolution
#define CLOSE_ENOUGH .001

// ----structs----

struct sdfRet
{
	float sdf;
    float id;
};
    
struct sphere
{
	vec3 c;
    float r;
};
  
struct ray
{
	vec3 origin;
    vec3 direction;
};
    
struct dirLight
{
	vec3 direction;
    float intensity;
    vec3 color;
    float ambient;
};


// ----distance functions----

    
float sphDist(vec3 p, sphere s)
{
	return distance(p, s.c) - s.r;
}

float sdCapsule( vec3 p, vec3 a, vec3 b, float r )
{
    vec3 pa = p - a, ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h ) - r;
}

//from iq : https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float smin( float d1, float d2, float k ) 
{
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h); 
}

//from iq : https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float smax( float d1, float d2, float k ) 
{
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h); 
}


// ----lighting----


vec3 diffuse(vec3 normal, dirLight l, float s)
{
	return max(dot(normal, l.direction)*l.intensity*s, l.ambient)*l.color;
}
    
vec3 idtocol(float id)
{
    vec3 col = vec3(.2,.9,.2);
    
    if(id > .5) col = vec3(.1,.1,.6);
    if(id > 1.5) col = vec3(1.0);
    if(id > 2.5) col = vec3(.1);
    
    return col;
}    

sdfRet frog(vec3 point)
{
    //body
    float id = .0;
    float dist = sphDist(point, sphere(vec3(.0,.05,.0), .25 ));
    
    //shoulders
    dist = smin(dist, sphDist(point, sphere(vec3(.34,-.12,-.1), .08)), .2);
    
    ////lower body
    dist = smin(dist, sphDist(point, sphere(vec3(.0,.02,.3), .1)), .3);
    
    ////thighs
    dist = smin(dist, sphDist(point, sphere(vec3(.24,-.12, .34), .08)), .2);
    
    //head
    dist = smin(dist, sphDist(point, sphere(vec3(.0,.04,-.25), .22)), .1);
    
    dist = max(-max(sdCapsule(point, vec3(-1.,-.04,-.5),vec3(1.,-.04,-.5),.1),point.y-.01),dist);
    
    //eyes
    float distEyes = sphDist(point, sphere(vec3(.15,.11,-.3), .14));
    
    if(dist > distEyes) id = 2.0;
    dist = min(dist,distEyes);
    
    //iris
    float distIris = sphDist(point, sphere(vec3(.19,.11,-.32), .1));
    if(dist > distIris) id = 3.0;
    dist = min(dist,distIris);
    
    return sdfRet(dist, id);
}

sdfRet map(vec3 point)
{
    
    point.z *=-1.0;
    point = vec3(abs(point.x),point.y,point.z);
    
    sdfRet d = frog(point);
    
    //plane
    float d2 = point.y+.2;
    
    sdfRet r = sdfRet(d2,1.0);
    if(d.sdf < d2) r = d;
    return r;
}

//from iq https://www.iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec3 calcNormal( in vec3 p )
{
    const vec2 k = vec2(1,-1);
    return normalize( k.xyy*map( p + k.xyy*CLOSE_ENOUGH).sdf + 
                      k.yyx*map( p + k.yyx*CLOSE_ENOUGH).sdf + 
                      k.yxy*map( p + k.yxy*CLOSE_ENOUGH).sdf + 
                      k.xxx*map( p + k.xxx*CLOSE_ENOUGH).sdf );
}


float shadows(ray r)
{
    float dist = CLOSE_ENOUGH;
    float returnval = 1.0;
    
	for(int i = 0; i < 100; i++)
    {
    	vec3 rp = r.origin + r.direction *dist;
        sdfRet ret = map(rp);
        returnval = min(16.*ret.sdf/dist, returnval);
        dist += ret.sdf;
        if(dist > 20.0) break;
    }
    
    return returnval;
}

vec3 lighting(vec3 point)
{
	dirLight sun = dirLight(normalize(vec3(.5,.5, .2)), 1.0, vec3(1.0,.9,.8),.02);
    dirLight horizon = dirLight(vec3(.0,1.,.0), .05, vec3(.3,.3,.8),.0);
    dirLight bounce = dirLight(vec3(.0,-1.0,.0), .02, vec3(.1,.1,.6),.0);
    
    vec3 normal = calcNormal(point);
    
    ray shadowRay = ray(point+normal*CLOSE_ENOUGH,sun.direction);    
    vec3 col = diffuse(normal,sun, shadows(shadowRay));
    
    col += diffuse(normal,horizon,1.0);
    
    col += diffuse(normal,bounce,1.0);
    
    return col;
    
}

const int aa = 2;

void main()
{
    
    vec3 total = vec3(.0);
    //MSAA
    for(int i = 0; i <aa;i++)
    for(int j = 0; j <aa;j++)
    {
    	//offset the uv for MSAA
    	vec2 uv = (gl_FragCoord.xy+vec2(float(i),float(j))/float(aa)-.5)/iResolution.xy - vec2(.5);
    	uv.x *= iResolution.x/iResolution.y;
    	
    	float mousex = .4;
    	
        //camera calculations
    	vec3 target = vec3(.0,.0,.0);
    	vec3 rayOrigin = target + vec3(1.5*sin(mousex),.2,1.5*cos(mousex));
    	vec3 front = normalize(target - rayOrigin);
    	vec3 right = normalize(cross(front, vec3(0.,1.,0.)));
    	vec3 up = normalize(cross(right, front));
    	vec3 rayDir = normalize(uv.x*right+uv.y*up+1.0*front);
    	
    	ray currentRay = ray(rayOrigin, rayDir);
    	
        //sky color
    	vec3 col = vec3(.4,.4,.9)-vec3(.4,.4,.9)*currentRay.direction.y;
    	
        //raymarching
    	float dist = .0;
    	for(int i = 0; i < 400; i++)
    	{
            //point at which we'll sample distances
    		vec3 rayPoint = currentRay.origin + currentRay.direction *dist;
    	    
            //sampling of distances
            sdfRet ret = map(rayPoint);
            
            //smaller than our epsilon : hit!
    	    if(ret.sdf < CLOSE_ENOUGH)
    	    {
    	        col = lighting(rayPoint)*idtocol(ret.id);
    	        break;
    	    }
    	    if(dist > 20.0) break;
    	    dist += ret.sdf;
    	}
     
    	total += col;
    }
    total /= float(aa*aa);
    
    // Output to screen
    outColor = vec4(pow(total,vec3(1./2.2)),1.0);
}