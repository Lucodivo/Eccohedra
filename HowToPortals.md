# A METHOD FOR DRAWING PORTALS

### Method Portal Definition
- A doorway to a separate 3D environment or "scene"

### Steps
1) Clear stencil buffer to *portal_base_mask*. 
    - *portal_base_mask* is some value in the range of [1, (2<sup>n</sup> - *max_portal_recursion* - 1)]
      - *n* is the number of bits per pixel in the stencil buffer.
      - *max_protal_recursion* is the maximum depth of portals within portals that you wish for your renderer support.
2) Draw geometry of current scene.
3) Recursion Base Case: If the current level of recursion is equal to *max_portal_recursion*, do nothing and pop one level 
    of recursion, returning to the recursive step that lead us here or ending the algorithm all together if *max_portal_recursion*
    is equal to zero. If we have not reached *max_portal_recursion*, continue to the next step.
4) Draw each portal in the scene to the depth buffer. 
   - Definitely do **NOT** draw to the stencil buffer. Drawing to color is find and can potentially help in debugging.
5) For each portal in the scene...
   1) Draw the portal to increment the stencil value by 1, where the portal's depth is both equal to that stored in
        the depth buffer and the stencil is equal to *scene_current_mask*.
      - *scene_current_mask* is equal to *portal_base_mask* plus the level of recursion (starting at level zero).
   2) Draw the portal to clear the depth buffer, where the portal's stencil value is equal to *portal_current_mask*.
      - *portal_current_mask* is *scene_current_mask* plus one. (produced by the increment of the last step)
   3) Create a unique oblique projection matrix made specifically for the portal and supply it to the renderer. 
      - The purpose of this projection matrix is to render only what is **beyond** a virtual window.
        If the same projection matrix is used for the scene containing the portal, the "virtual window" will act 
        simply as a sliver of another scene. The oblique projection matrix clips any geometry between the camera and the
        portal, preventing anything in the scene beyond the portal to rendered in front of the portal.
      - See [references](#References) for an in-depth general explanation of an oblique projection matrix.
      - See function [*obliquePerspective*](shared_cpp/noop_math/noop_math.cpp#L1095) in project source for this 
         project's implementation.
   4) Draw geometry of scene beyond portal using the oblique projection matrix, where the stencil value is equal
     to *portal_current_mask*.
   5) Recursive Step: Start at step 2 for the scene beyond the portal. This increments the level of recursion by one.
   6) Draw the portal to clear the stencil value to zero.
6) END: Pop one level of recursion, returning to the recursive step that lead us here. Or, if *scene_current_mask* equals
    *portal_base_mask*, the algorithm is complete and there is no work left to be done.

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