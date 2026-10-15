#version 450

// The vertex buffer's inputs, in the order createPipeline's attributeSizes lists them.
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

// Sent with Vulkan.pushConstants() before each draw.
layout(push_constant) uniform Push {
    mat4 transform;
} push;

layout(location = 0) out vec3 vColor;

void main() {
    gl_Position = push.transform * vec4(position, 1.0);
    vColor = color;
}
