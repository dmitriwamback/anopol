package core;

import org.lwjgl.glfw.GLFWVidMode;
import org.lwjgl.opengl.GL;
import org.lwjgl.opengl.GL11;
import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.system.MemoryStack.*;
import static org.lwjgl.system.MemoryUtil.*;

import java.util.HashMap;
import java.util.Map;

import org.lwjgl.system.MemoryStack;
import core.shaders.Shader;


public class AnopolCore {
    
    public static long window;

    public static Map<String, Shader> shaders = new HashMap<String,Shader>();

    public void begin() {
        if (!glfwInit()) throw new IllegalStateException("Unable to initialize GLFW");

        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

        try (MemoryStack stack = stackPush()) {

            GLFWVidMode vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());

            window = glfwCreateWindow(vidmode.width(), vidmode.height(), "Anopol", NULL, NULL);
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glfwShowWindow(window);

        loop();
    }

    private void loop() {
        GL.createCapabilities();

        shaders.put("TILESHADER",       Shader.createShader("tile"));
        shaders.put("BLANKSHADER",      Shader.createShader("blank"));
        shaders.put("SKYBOX",           Shader.createShader("skybox"));
        shaders.put("SHADOWSHADER",     Shader.createShader("shadow"));
        shaders.put("NORMAL",           Shader.createShader("normal"));

        while (!glfwWindowShouldClose(window)) {
            GL11.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            GL11.glClear(GL11.GL_COLOR_BUFFER_BIT | GL11.GL_DEPTH_BUFFER_BIT);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
}
