# anopol
<p>Anopol is a Vulkan graphics engine used mainly to experiment with the Vulkan API. Over time, I will increasingly add more features into this project as to have a base for creating future games.</p>
<p>Current features:</p>
<ul>
  <li>Basic rendering + depth testing</li>
  <li>Instanced rendering</li>
  <li>Uniform buffer + push constants</li>
  <li>Collision detection (Gilbert-Johnson-Keerthi (GJK) + Expanding Polytope Algorithm (EPA))</li>
</ul>
<p>Future features:</p>
<ul>
  <li>Cascaded Shadow Mapping (CSM) / Parallel-Split Shadow Mapping (PSSM)</li>
  <li>Texture sampling</li>
  <li>Off-screen rendering</li>
  <li>Approximate Convex Decomposition</li>
  <li>Combining meshes (batching)</li>
</ul>
<p>Necessary libraries:</p>
<ul>
  <li>Vulkan SDK - Core functionality for the Vulkan API</li>
  <li>GLFW - Framework for rendering to the screen</li>
  <li>GLM - OpenGL Math library (Vectors, Matrices, etc.)</li>
  <li>Assimp - Model loader</li>
</ul>
<p>This engine currently has the skeleton for rendering/displaying objects to the screen. However, I hope to make this project more profound by implementing more sophisticated algorithms.</p>
<p>Future implementations (individual repositories as to not get lost in one project):</p>
<ul>
  <li>
    <a href="https://github.com/dmitriwamback/GJK">GJK Algorithm for collision detection</a>
  </li>
  <li>
    <a href="https://github.com/dmitriwamback/i-k">Inverse kinematics</a>
  </li>
  <li>
    <a href="https://github.com/dmitriwamback/multiplayer-v2">Mockup multiplayer / sockets</a>
  </li>
</ul>

