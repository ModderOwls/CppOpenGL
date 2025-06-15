#version 330 core

in vec3 FragPos;
in vec3 worldPosition;
in vec3 normal;
in vec2 texCoord;

out vec4 FragColor;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;
uniform float time;

uniform sampler2D textureNormal;
uniform sampler2D textureTerrain;


vec3 lerp(vec3 a, vec3 b, float t)
{
    return a + (b - a) * t;
}

void main() 
{
    vec3 newNormal = normal;
    vec2 sizedTexCoord = texCoord * 33;
    vec3 viewDirection = normalize(worldPosition.rgb - cameraPosition);
    float dist = length(worldPosition.xyz - cameraPosition);

    vec3 color = vec3(0.0, 0.3, 0.7);

    //Add a 'wave' like effect.
    vec2 waveTexCoord = sizedTexCoord + vec2(sin(time + sizedTexCoord.x * 10.0) * 0.02, cos(time + sizedTexCoord.y * 10.0) * 0.02);

    //Apply the normal to the uv.
    vec3 mappedNormal = texture(textureNormal, waveTexCoord * 3).rgb * 2.0 - 1.0;
    newNormal = normalize(newNormal + mappedNormal * 0.2); // Slightly distort surface
    
    //Add a fresnel effect.
    float fresnel = pow(1.0 - max(dot(newNormal, viewDirection), -0.3), 5.0);
    //fresnel = clamp(fresnel, 0.0, 0.5);

    //Add specular.
    float spec = pow(max(dot(reflect(lightDirection, newNormal), -viewDirection), -0.0), 32.0);

    //Combine them.
    color = color + spec;


    //Animated foam UV
    vec2 foamTexCoord = texCoord + vec2(sin(time + texCoord.x * 1.0) * 0.005, cos(time + texCoord.y * 1.0) * 0.005);
    vec3 foam = texture(textureTerrain, foamTexCoord).rgb;
    float foamAmount = (min(foam.b, 0.62) - 0.62) * 8;

    //Apply foam
    color = mix(color, vec3(0), foamAmount);
    

    //Add a fog effect.
    float fog = pow(clamp((dist - 250) / 1000, 0, 1), 2);
    
    vec3 topColor = vec3(68.0 / 255.0, 118.0 / 255.0, 189.0 / 255.0);
    vec3 botColor = vec3(188.0 / 255.0, 214.0 / 255.0, 231.0 / 255.0);

    vec3 fogColor = lerp(botColor, topColor, max(viewDirection.y, 0.0));

    color = lerp(color, fogColor, fog);


    FragColor = vec4(color, 0.9);
}