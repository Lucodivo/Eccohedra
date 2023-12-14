# A METHOD FOR DRAWING PORTALS

### Method Portal Definition
- A doorway to a separate 3D environment or "scene"

### Method Limitations
- 1 bit is used per scene (potential portal destination)
    - There are typically only 8 bpp in a stencil buffer, meaning that the current implementation is limited to 
    8 scenes. Though one should be able to overcome these limitations by interpreting bits differently depending
    on the context of the program.
        - Ex: Given the context of the current scene, one can differ in interpretations of bits in the stencil buffer.
        Instead of having some global meaning of each bit.

### Steps
1) Draw geometry of current scene.
2) Recursion Base Case: If we have reached our defined portal recursive depth, we have completed. End of algorithm.
3) For each portal, draw them to the stencil buffer. Turning on the bit that represents the scene of that portals destination.
   - In OpenGL, specific bits are turned on using... 
        - glStencilOp(ref = 0xFF, stencilMask = portalSourceBitMask), which makes it so the stencil test is:  
          0xFF & portalSourceBitMask == stencilBufferFragmentBits & portalSourceBitMask  
          Which allows a portal to be drawn only in a place where the portals home scene bit has been written.
        - glStencilMask(mask = portalDestinationBitMask), which limits the writing of bits to only the bit represented by portalDestinationBitMask.
        - glStencilFunc(GL_KEEP, GL_KEEP, GL_REPLACE), which writes glStencilOp's ref (0xFF) ANDed with glStencilMasks's bits to the stencil buffer only when a 
          fragment passes both the depth and stencil tests. The result is turning on the destination scene's bit and leaving other bits unaffected.
4) For each portal, redraw them to the stencil buffer, taking advantage of the depth buffer now handling overlapping portals,
  and turn off all bits that do not represent the scene of the portal's destination.
   - In OpenGL, specific bits are turned off using...
        - glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO), which says if we pass the stencil tests and the depth test, we want to zero all the bits.
        - glStencilMask(mask = ~portalDestinationBitMask), which limits the zeroing to all but the destination scene's bit.
5) For each portal, redraw them in a way that clears the depth buffer for the region covered by the portal.
   - In OpenGL, specific depth bits are cleared using...
         - glStencilFunc(GL_EQUAL, 0xFF, portalDestinationBitMask), which makes the stencil test only pass where the portalsDestinationBit has been set
         - glDepthFunc(GL_ALWAYS), which makes the depth test always pass
         - gl_FragDepth = 1.0f, which always writes the depth of the fragment to the furthest value (this is GLSL code set in the fragment shader)
6) For each portal...
    1) Create a unique oblique projection matrix made specifically for the portal and supply it to the renderer.
    2) Draw geometry of scene beyond portal using the oblique projection matrix
    3) Recursive Step: Go to step 2 for the scene beyond this portal.

### NOTES 

#### How Does This Work Again?: OpenGL's Stencil Buffer
- glEnable/Disable(GL_STENCIL_TEST) enable and disable not only testing for the stencil but also writing to it.
- glStencilOp(on_fail, on_pass_but_depth_fail, on_all_pass) determines what values are written to the stencil buffer with the help of glStencilMask(x)
    - If on_pass_but_depth_fail is GL_ZERO, it zeros out all bits of glStencilMask's mask for a fragment passing the stencil test but failing the depth test
- glStencilMask(mask) is a bit mask applied to stencil values produced by glStencilOp() before written to stencil buffer
- glStencilFunc(func, ref, mask) determines whether a given pixel passes a stencil test.
    - if func is GL_EQUAL, the following test performed is:
        - ref & mask == existing_stencil_value & mask
    - if glStencilOp uses GL_REPLACE, the value placed into the buffer is ref & glStencilMask (not the mask argument of this function)

#### References
- [Oblique View Frustum Depth Projection and Clipping by Eric Lengyel (Terathon Software](https://www.terathon.com/lengyel/Lengyel-Oblique.pdf)
- [LearnOpenGL's Stencil Testing by Joey de Vries](https://learnopengl.com/Advanced-OpenGL/Stencil-testing)