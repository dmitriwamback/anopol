package core.shaders;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

import org.joml.Vector2i;
import org.lwjgl.opengl.GL41;

public class Shader {
    
    private int programID;

    public static Shader createShader(String shaderPath) {
        Shader shader = new Shader();

        Vector2i shaderIDs = Shader.loadShaderSource(shaderPath);

        shader.programID = GL41.glCreateProgram();
        GL41.glAttachShader(shader.programID, shaderIDs.x);
        GL41.glAttachShader(shader.programID, shaderIDs.y);
        GL41.glLinkProgram(shader.programID);

        GL41.glDeleteShader(shaderIDs.x);
        GL41.glDeleteShader(shaderIDs.y);

        return shader;
    }

    private static Vector2i loadShaderSource(String path) {

        int vertShader = GL41.glCreateShader(GL41.GL_VERTEX_SHADER);
        int fragShader = GL41.glCreateShader(GL41.GL_FRAGMENT_SHADER);

        StringBuilder vStringBuilder = new StringBuilder();
        StringBuilder fStringBuilder = new StringBuilder();

        try (BufferedReader br = new BufferedReader(new FileReader("src/core/shaders/sources/"+path+"/vMain.glsl"))) {
            String line;
            while ((line = br.readLine()) != null) {
                vStringBuilder.append(line).append("\n");
            }
            GL41.glShaderSource(vertShader, vStringBuilder);
        }
        catch (IOException e) {}

        try (BufferedReader br = new BufferedReader(new FileReader("src/core/shaders/sources/"+path+"/fMain.glsl"))) {
            String line;
            while ((line = br.readLine()) != null) {
                fStringBuilder.append(line).append("\n");
            }
            GL41.glShaderSource(fragShader, fStringBuilder);
        }
        catch (IOException e) {}

        GL41.glCompileShader(vertShader);
        GL41.glCompileShader(fragShader);

        if (GL41.glGetShaderi(vertShader, GL41.GL_COMPILE_STATUS) == GL41.GL_FALSE) {
            System.err.println("Failed to compile shader: " + GL41.glGetShaderInfoLog(vertShader));
        }
        if (GL41.glGetShaderi(fragShader, GL41.GL_COMPILE_STATUS) == GL41.GL_FALSE) {
            System.err.println("Failed to compile shader: " + GL41.glGetShaderInfoLog(fragShader));
        }

        Vector2i shaderIDs = new Vector2i(vertShader, fragShader);
        return shaderIDs;
    }
}
