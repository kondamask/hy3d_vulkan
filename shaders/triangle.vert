#version 450

layout (location = 0) out vec3 outColor;

void main()
{
	const vec3 positions[] = vec3[](vec3(-1.0f,  0.0f,  0.0f),
                                    vec3(-0.5f, -1.0f,  0.0f),
                                    vec3( 0.0f,  0.0f,  0.0f));
    
    /*                                     
                                        vec3( 0.0f,  0.0f,  0.0f),
                                        vec3( 0.5f, -1.0f,  0.0f),
                                        vec3( 1.0f,  0.0f,  0.0f));
         */
    
    const vec3 colors[] = vec3[](vec3(1.0f, 1.0f, 0.0f), //red
                                 vec3(0.0f, 1.0f, 1.0f), //green
                                 vec3(1.0f, 0.0f, 1.0f)); //blue
    
    /* 
                                     vec3(1.0f, 0.0f, 0.0f), //red
                                     vec3(0.0f, 1.0f, 0.0f), //green
                                     vec3(0.0f, 0.0f, 1.0f) //blue
                                     );
     */
    
	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
    outColor = colors[gl_VertexIndex];
}