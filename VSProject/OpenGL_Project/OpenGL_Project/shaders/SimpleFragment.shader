#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 texCoord;
in vec3 normal;
in mat3 tbn;
in vec3 worldPosition;

uniform sampler2D texture1;
uniform sampler2D texture1Normal;
uniform sampler2D texture2;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;


void main()
{
    //Normal map.
    vec3 normal = texture(texture1Normal, texCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    //Scale down normal.
    normal.rg = normal.rg * 0.8f;
    normal = normalize(normal);

    //Transform normal using tbn.
    normal = tbn * normal;

    vec3 lightDirection = normalize(worldPosition - lightPosition);

    //Specular data.
    vec3 viewDirection = normalize(worldPosition - cameraPosition);
    vec3 reflectDirection = normalize(reflect(lightDirection, normal));

    //Lighting.
    float lightValue = max(-dot(normal, lightDirection), 0.0);
    float specular = pow(max(-dot(reflectDirection, viewDirection), 0.0), 48);

    //Seperate rgba and rgb use.
    vec4 output = vec4(ourColor, 1.0f) * mix(texture(texture1, texCoord), texture(texture2, texCoord), 0.2);
    output.rgb = output.rgb * min(lightValue + 0.1, 1.0) + specular * output.rgb + specular * .4f;

    FragColor = output; //vec4(texCoord, 0.0f, 1.0f);
}