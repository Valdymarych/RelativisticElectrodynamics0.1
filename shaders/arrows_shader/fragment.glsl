#version 430
in vec4 color_out;
out vec4 outColor;
void main() {
    outColor = vec4(color_out);
}