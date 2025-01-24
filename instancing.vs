#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 instancePosition;
layout(location = 2) in float instanceRadius;
layout(location = 3) in vec4 instanceColor;

uniform mat4 mvp;

out vec4 fragColor;

void main() {
    // Scale the sphere by its radius and translate it to its position
    vec3 scaledPosition = vertexPosition * instanceRadius;
    vec3 finalPosition = scaledPosition + instancePosition;

    gl_Position = mvp * vec4(finalPosition, 1.0);
    fragColor = instanceColor;

    // Debug output
    // if (gl_VertexID == 0) {
    //     printf("Final position: %f, %f, %f\n", finalPosition.x, finalPosition.y, finalPosition.z);
    //     printf("Color: %f, %f, %f, %f\n", instanceColor.r, instanceColor.g, instanceColor.b, instanceColor.a);
    // }
}
