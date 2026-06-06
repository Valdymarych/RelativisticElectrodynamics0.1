out vec4 outColor;
in vec3 color_out;
void main() {
    vec3 clamped_color = clamp(color_out,0.,1.);
    outColor = vec4(clamped_color, 1.0);
}