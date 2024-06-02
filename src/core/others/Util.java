package core.others;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.joml.Vector2f;
import org.lwjgl.glfw.GLFW;
import org.lwjgl.glfw.GLFWCursorPosCallbackI;
import org.lwjgl.glfw.GLFWWindowSizeCallbackI;

import core.AnopolCore;
import core.ui.SelectablePanel;

public class Util {
    
    public static Vector2f mousePosition, windowSize;
    public static void initialize() {

        GLFW.glfwSetCursorPosCallback(AnopolCore.window, new GLFWCursorPosCallbackI() {

            @Override public void invoke(long window, double xpos, double ypos) {
                mousePosition = new Vector2f((float)xpos, (float)ypos);
            } 
        });

        GLFW.glfwSetWindowSizeCallback(AnopolCore.window, new GLFWWindowSizeCallbackI() {

            @Override public void invoke(long window, int width, int height) {
                windowSize = new Vector2f(width, height);
            }
        });
    }

    public static boolean isInBoundaries(Vector2f bottomLeft, Vector2f topRight, Vector2f pointOnScreen) {
        return (pointOnScreen.x > bottomLeft.x && pointOnScreen.x < topRight.x &&
                pointOnScreen.y > bottomLeft.y && pointOnScreen.y < topRight.y);
    }



    public static List<Float> createVertices_CLICKABLEPANE(Vector2f absoluteSize, Vector2f offset) {

        float normalizedxscale = (absoluteSize.x / (float)windowSize.x) * windowSize.x * 0.0007f;
        float normalizedyscale = (absoluteSize.y / (float)windowSize.y) * windowSize.x * 0.0007f;

        float xo = (offset.x / windowSize.x) * windowSize.x * 0.0007f;
        float yo = (offset.x / windowSize.y) * windowSize.x * 0.0007f;

        return new ArrayList<Float>(
            Arrays.asList(-normalizedxscale + xo,  normalizedyscale + yo, 0.0f, 1.0f,
                           normalizedxscale + xo,  normalizedyscale + yo, 1.0f, 1.0f,
                          -normalizedxscale + xo, -normalizedyscale + yo, 0.0f, 0.0f,

                          -normalizedxscale + xo, -normalizedyscale + yo, 0.0f, 0.0f,
                           normalizedxscale + xo, -normalizedyscale + yo, 1.0f, 0.0f,
                           normalizedxscale + xo,  normalizedyscale + yo, 1.0f, 1.0f));
    }



    public static Vector2f createVertexNormalizedScale(Vector2f absoluteSize, Vector2f vector2f) {
        
        float normalizedxscale = (absoluteSize.x / (float)windowSize.x) * windowSize.x * 0.0007f;
        float normalizedyscale = (absoluteSize.y / (float)windowSize.y) * windowSize.x * 0.0007f;

        return new Vector2f(normalizedxscale, normalizedyscale);
    }



    public static Vector2f toScreenCoordinates(Vector2f coordinates) {

        return new Vector2f(
            coordinates.x / 0.0007f,
            coordinates.y * windowSize.y / (0.0007f * windowSize.x)
        );
    }



    public static List<Float> computeScreenCollisionDetection(SelectablePanel panel, Vector2f normalizedScale, float yOffset) {

        List<Float> collidedVertices = new ArrayList<Float>();

        float collidedY = panel.vertexData.localPosition.y - (-panel.ySliderOffset);

        if (normalizedScale.y + panel.vertexData.localPosition.y - yOffset - (-panel.ySliderOffset) > 1) {
            panel.vertexData.localPosition.y = 1 - normalizedScale.y;
            panel.topRight.y = 1;
            panel.bottomLeft.y = -normalizedScale.y;

            collidedVertices = createVertices_CLICKABLEPANE(
                panel.absoluteSize, 
                toScreenCoordinates(new Vector2f(panel.vertexData.localPosition.x, 
                                                 1 - normalizedScale.y - panel.ySliderOffset)));
        }


        return collidedVertices;
    }
}
