#version 430
layout(points) in;
layout(triangle_strip, max_vertices=21) out;
in vec3 arrow_1[];
in vec4 color_1[];
in vec3 arrow_2[];
in vec4 color_2[];
in vec3 arrow_3[];
in vec4 color_3[];
out vec4 color_out_frag;
void main() {
    color_out_frag=color_1[0];
    float width = -0.05;
    vec4 pointPos = gl_in[0].gl_Position;
    vec3 arrow = arrow_1[0];
    vec2 perp = vec2(arrow.y,-arrow.x)*width;
    vec2 arrow_2d = arrow.xy;
    float arrow_z = arrow.z;
    gl_Position = pointPos + vec4(1.5*arrow_2d, 0., 0.);
    EmitVertex();
    gl_Position = pointPos + vec4(-3.*perp+arrow_2d, 0., 0.); 
    EmitVertex();
    gl_Position = pointPos + vec4(3.*perp+arrow_2d, 0., 0.);
    EmitVertex();
    EndPrimitive();
    gl_Position = pointPos + vec4(perp, 0., 0.);
    EmitVertex();
    gl_Position = pointPos + vec4(perp+arrow_2d, 0., 0.0);
    EmitVertex();
    gl_Position = pointPos + vec4(-perp, 0., 0.0);
    EmitVertex();
    gl_Position = pointPos + vec4(-perp+arrow_2d, 0., 0.0); 
    EmitVertex();
    EndPrimitive();



    color_out_frag=color_2[0];
    width = -0.05;
    pointPos = gl_in[0].gl_Position;
    arrow = arrow_2[0];
    perp = vec2(arrow.y,-arrow.x)*width;
    arrow_2d = arrow.xy;
    arrow_z = arrow.z;
    gl_Position = pointPos + vec4(1.5*arrow_2d, 0., 0.);
    EmitVertex();
    gl_Position = pointPos + vec4(-3.*perp+arrow_2d, 0., 0.); 
    EmitVertex();
    gl_Position = pointPos + vec4(3.*perp+arrow_2d, 0., 0.);
    EmitVertex();
    EndPrimitive();
    gl_Position = pointPos + vec4(perp, 0., 0.);
    EmitVertex();
    gl_Position = pointPos + vec4(perp+arrow_2d, 0., 0.0);
    EmitVertex();
    gl_Position = pointPos + vec4(-perp, 0., 0.0);
    EmitVertex();
    gl_Position = pointPos + vec4(-perp+arrow_2d, 0., 0.0); 
    EmitVertex();
    EndPrimitive();



    color_out_frag=color_3[0];
    width = -0.05;
    pointPos = gl_in[0].gl_Position;
    arrow = arrow_3[0];
    perp = vec2(arrow.y,-arrow.x)*width;
    arrow_2d = arrow.xy;
    arrow_z = arrow.z;
    gl_Position = pointPos + vec4(1.5*arrow_2d, 0., 0.);
    EmitVertex();
    gl_Position = pointPos + vec4(-3.*perp+arrow_2d, 0., 0.); 
    EmitVertex();
    gl_Position = pointPos + vec4(3.*perp+arrow_2d, 0., 0.);
    EmitVertex();
    EndPrimitive();
    gl_Position = pointPos + vec4(perp, 0., 0.);
    EmitVertex();
    gl_Position = pointPos + vec4(perp+arrow_2d, 0., 0.0);
    EmitVertex();
    gl_Position = pointPos + vec4(-perp, 0., 0.0);
    EmitVertex();
    gl_Position = pointPos + vec4(-perp+arrow_2d, 0., 0.0); 
    EmitVertex();
    EndPrimitive();

}