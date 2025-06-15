#version 330 core
out vec4 FragColor;
  
in vec2 texCoord;
in vec3 worldPosition;

uniform sampler2D texture1;
uniform sampler2D texture1Normal;

uniform sampler2D dirt, sand, grass, rock, snow;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;


vec3 lerp(vec3 a, vec3 b, float t)
{
    return a + (b - a) * t;
}

void main()
{
    //Normal map.
    vec3 normal = texture(texture1Normal, texCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal.gb = normal.bg;
    normal.r = -normal.r;
    normal.b = -normal.b;

    vec3 fixedLightDirection = lightDirection;
    fixedLightDirection.z *= -1;

    //Specular data.
    vec3 viewDirection = normalize(worldPosition.rgb - cameraPosition);
    //vec3 reflectDirection = normalize(reflect(lightDirection, normal));

    //Lighting.
    float lightValue = max(-dot(normal, fixedLightDirection), 0.0);
    //float specular = pow(max(-dot(reflectDirection, viewDirection), 0.0), 48);

    //Build color.
    float y = worldPosition.y;

    float ds = clamp((y - 50) / 10, -1, 1) * .5 + .5;
    float sg = clamp((y - 75) / 10, -1, 1) * .5 + .5;
    float gr = clamp((y - 125) / 10, -1, 1) * .5 + .5;
    float rs = clamp((y - 200) / 10, -1, 1) * .5 + .5;

    float dist = length(worldPosition.xyz - cameraPosition);
    float texCoordsLerp = clamp((dist - 250) / 150, -1, 1) * .5 + .5;

    vec3 dirtColorClose = texture(dirt, texCoord * 100).rgb;
    vec3 sandColorClose = texture(sand, texCoord * 100).rgb;
    vec3 grassColorClose = texture(grass, texCoord * 100).rgb;
    vec3 rockColorClose = texture(rock, texCoord * 100).rgb;
    vec3 snowColorClose = texture(snow, texCoord * 100).rgb;    
    
    vec3 dirtColorFar = texture(dirt, texCoord * 10).rgb;
    vec3 sandColorFar = texture(sand, texCoord * 10).rgb;
    vec3 grassColorFar = texture(grass, texCoord * 10).rgb;
    vec3 rockColorFar = texture(rock, texCoord * 10).rgb;
    vec3 snowColorFar = texture(snow, texCoord * 10).rgb;

    vec3 dirtColor = lerp(dirtColorClose, dirtColorFar, texCoordsLerp);
    vec3 sandColor = lerp(sandColorClose, sandColorFar, texCoordsLerp);
    vec3 grassColor = lerp(grassColorClose, grassColorFar, texCoordsLerp);
    vec3 rockColor = lerp(rockColorClose, rockColorFar, texCoordsLerp);
    vec3 snowColor = lerp(snowColorClose, snowColorFar, texCoordsLerp);

    vec3 diffuse = lerp(lerp(lerp(lerp(dirtColor, sandColor, ds), grassColor, sg), rockColor, gr), snowColor, rs);

    float fog = pow(clamp((dist - 250) / 1000, 0, 1), 2);
    
    vec3 topColor = vec3(68.0 / 255.0, 118.0 / 255.0, 189.0 / 255.0);
    vec3 botColor = vec3(188.0 / 255.0, 214.0 / 255.0, 231.0 / 255.0);

    vec3 fogColor = lerp(botColor, topColor, max(viewDirection.y, 0.0));


    //Seperate rgba and rgb use.
    vec4 output = vec4(lerp(diffuse * min(lightValue + 0.1, 1.0), fogColor, fog), 1.0);// + specular * output.rgb + specular * .4f;

    FragColor = output; //vec4(texCoord, 0.0f, 1.0f);
}