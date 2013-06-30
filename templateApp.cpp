/*

Book:      	Game and Graphics Programming for iOS and Android with OpenGL(R) ES 2.0
Author:    	Romain Marucchi-Foino
ISBN-10: 	1119975913
ISBN-13: 	978-1119975915
Publisher: 	John Wiley & Sons	

Copyright (C) 2011 Romain Marucchi-Foino

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone who either own or purchase a copy of
the book specified above, to use this software for any purpose, including commercial
applications subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that
you wrote the original software. If you use this software in a product, an acknowledgment
in the product would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented
as being the original software.

3. This notice may not be removed or altered from any source distribution.

*/

#include "templateApp.h"

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw,
                            templateAppToucheBegan,
                            templateAppToucheMoved,
                            templateAppToucheEnded};

#define VERTEX_SHADER ( char * )"vertex.glsl"

#define FRAGMENT_SHADER ( char * )"fragment.glsl"

#define DEBUG_SHADERS 1

PROGRAM *program = NULL;

MEMORY *m = NULL;

void templateAppInit( int width, int height )
{
	atexit( templateAppExit );

	GFX_start();

	glViewport( 0.0f, 0.0f, width, height );

   // Now use the GFX_set_matrix mode function to tell the GFX implementation to focus the projection matrix in order to setup a 2D projection
    GFX_set_matrix_mode(PROJECTION_MATRIX);
    
    float half_width = (float) width/2;
    float half_height = (float) height/2;
    
    //make sure that hte current matrix is clean
    GFX_load_identity();
    
    //set the projection matrix right in the midle of the screeen, using 1:1 ratio between GL units and screen pixels
    GFX_set_orthographic_2d(-half_width, half_width, -half_height, half_height);
    
    //translate the matrix to the bottom left of the screen because opengl uses the bottom left corner as (0,0)
    GFX_translate( -half_width, -half_height, 0.0f );
    
    //disable the depth buffer
    glDisable(GL_DEPTH_TEST);
    
    //disable the depth mask
    glDepthMask(GL_FALSE);
    
    //initialize the program variable pointer
    program = PROGRAM_init((char*)"default");
    
    //create a new vertex and fragment shader
    program->vertex_shader = SHADER_init( VERTEX_SHADER, GL_VERTEX_SHADER );
    program->fragment_shader = SHADER_init( FRAGMENT_SHADER, GL_FRAGMENT_SHADER );
    
    //load the content of the vertex shader in memory
    m = mopen( VERTEX_SHADER, 1 );
    
    if(m)
    {
        //compile the vertex shader code
        //we pass the memory buffer to the SHADER_compile function
        if( !SHADER_compile( program->vertex_shader, ( char * )m->buffer, DEBUG_SHADERS ) ) exit( 1 );
    }
    //free memory structure pointer m
    m=mclose(m);
    
    //load the fragment shader and compile it
    m = mopen( FRAGMENT_SHADER, 1 );
    if( m )
    {
        if( !SHADER_compile( program->fragment_shader, ( char * )m->buffer, DEBUG_SHADERS ) ) exit( 2 );
    }
    m = mclose( m );
    
    //execute the final linking phase
    if( !PROGRAM_link( program, DEBUG_SHADERS ) ) exit( 3 );
    
    /* Use the built-in GFX cross-platform API to print on the console (XCode) or LogCat (Eclipse) that the execution pointer passes the templateAppInit function. */
    console_print("templateAppInit screen size %d x %d ",width, height);
   
}


void templateAppDraw( void )
{
    //the vertices
    static const float POSITION[ 8 ] =
    {
        0.0f, 0.0f, // Down left (pivot point)
        1.0f, 0.0f, // Up left
        0.0f, 1.0f, // Down right
        1.0f, 1.0f // Up right
    };
    
    //the vertex colors
    static const float COLOR[ 16 ] = {
        1.0f /* R */, 0.0f /* G */, 0.0f /* B */, 1.0f /* A */,/* Red */
        0.0f, 1.0f, 0.0f, 1.0f, /* Green */
        0.0f, 0.0f, 1.0f, 1.0f, /* Blue */
        1.0f, 1.0f, 0.0f, 1.0f /* Yellow */
    };
    
    //clean the color buffer with a light gray
    glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
	
    glClear( GL_COLOR_BUFFER_BIT );
	/* Select the model view matrix. */
    GFX_set_matrix_mode( MODELVIEW_MATRIX );
    /* Reset it to make sure you are going to deal with a clean
     identity matrix. */
    GFX_load_identity();
    /* Scale the quad to be 100px by 100px. */
    GFX_scale( 100.0f, 100.0f, 0.0f );
    
    //make we have a valid shader program id
    if( program->pid )
    {
        // two temporary variables that you will use to hold the vertex attribute and uniform locations
        char attribute, uniform;
        glUseProgram( program->pid );
        
        //retrieve uniform variable from video memory
        uniform = PROGRAM_get_uniform_location( program,
                                     ( char * )"MODELVIEWPROJECTIONMATRIX");
        
        glUniformMatrix4fv(
        /* The location value of the uniform. */ uniform,
        /* How many 4x4 matrix */ 1,
        /* Specify to do not transpose the matrix. */GL_FALSE,
        /* Use the GFX helper function to calculate the result of the
         current model view matrix multiplied by the current projection matrix. */( float * )GFX_get_modelview_projection_matrix() );
        
        //retrieve the location of the vertex position
        attribute = PROGRAM_get_vertex_attrib_location( program, ( char * )"POSITION");
        
        glEnableVertexAttribArray( attribute );
        
        
        glVertexAttribPointer(
        /* The attribute location */
                              attribute,
        /* How many elements; XY in this case, so 2. */ 2,
        /* The variable type. */
                              GL_FLOAT,
        /* Do not normalize the data. */
                              GL_FALSE,
        /* The stride in bytes of the array delimiting the elements,
         in this case none. */ 0,
        /* The vertex position array pointer. */ POSITION );
        
        attribute = PROGRAM_get_vertex_attrib_location( program,
                                                       ( char * )"COLOR" ); glEnableVertexAttribArray( attribute );
        glVertexAttribPointer(attribute,
                              4,
                              GL_FLOAT,
                              GL_FALSE, 0,
                              COLOR );
        glDrawArrays(
        /* The drawing mode. */ GL_TRIANGLE_STRIP,
        /* Start at which index. */ 0,
        /* Start at which index. */ 4 );
    }
    //check the GFX_error as a safety measure
    GFX_error();
    console_print("templateAppDraw\n");
}

void templateAppToucheBegan( float x, float y, unsigned int tap_count )
{
    /* Print that the execution pointer enters the touche began function and print the touche XY value as well as the number of taps. */
    console_print( "templateAppToucheBegan,"
                  "touche: %f,%f"
                  "tap: %d\n", x, y, tap_count );
}

void templateAppToucheMoved( float x, float y, unsigned int tap_count )
{
    /* Print that the execution pointer enters the touche moved function and print the touche XY value as well as the number of taps. */
    console_print( "templateAppToucheMoved,"
                  "touche: %f,%f"
                  "tap: %d\n", x, y, tap_count );
}

void templateAppToucheEnded( float x, float y, unsigned int tap_count )
{
    /* Print that the execution pointer enters the touche ended function and print the touche XY value as well as the number of taps. */
    console_print( "templateAppToucheEnded,"
                  "touche: %f,%f"
                  "tap: %d\n", x, y, tap_count );}

void templateAppExit( void )
{
	printf("templateAppExit...\n");
	
    if( m ) m = mclose( m );
    
	if( program && program->vertex_shader ) 
		program->vertex_shader = SHADER_free( program->vertex_shader );

	if( program && program->fragment_shader )
		program->fragment_shader = SHADER_free( program->fragment_shader );

	if( program )
		program = PROGRAM_free( program );

	console_print("templateAppExit...\n");
}
