#version 330 core

// m: Model space
// w: World space
// e: Eye space

in vec3 eNormal;

uniform vec4 color;
uniform int bLighting;

out vec4 fragColor;

void main()
{
    float lighting = 1.0;
    
    if(bLighting != 0)
    {
        vec3 lightDirection = normalize(vec3(1.0, 2.0, 3.0));

        // Compute simple diffuse directional lighting with some ambient light
        float ambient = 0.2;
        float diffuse = max(0.0, dot(normalize(eNormal), lightDirection));
        lighting = 0.15 * ambient + 0.85 * diffuse;
    }

    // Modulate color with lighting and apply gamma correction
    fragColor = pow(lighting * color, vec4(1.0 / 2.1));
}

