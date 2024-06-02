package core.ui;

import org.joml.Vector2f;
import org.joml.Vector4f;
import org.lwjgl.glfw.GLFW;
import org.lwjgl.glfw.GLFWCursorPosCallbackI;

import core.AnopolCore;
import core.others.Texture;
import core.others.Util;
import core.shaders.Shader;
import core.types.RenderBuffer;
import core.types.Vertex;

public class SelectablePanel {
    public Vector2f absoluteSize, topRight, bottomLeft, hoverOffset, staticPosition;
    public Vertex vertexData;
    public RenderBuffer renderBuffer;

    public Texture uvSeal, mainTexture;
    public int preferredSelectMouseButton = GLFW.GLFW_MOUSE_BUTTON_LEFT;

    public boolean isProp, isSelected, containsUvSeal = false, isCurrentlyCollided;
    public float xSliderOffset, ySliderOffset;

    boolean isHovering(boolean withMouseInput) {
        if (isProp) return false;

        if (Util.isInBoundaries(bottomLeft, topRight, Util.mousePosition)) 
        {
            if (!isSelected) hoverOffset = vertexData.localPosition;
            isSelected = true;

            if (isCurrentlyCollided) {
                hoverOffset = vertexData.localPosition.sub(Util.mousePosition);
            }
            return true;
        }
        if (!withMouseInput && Util.isInBoundaries(bottomLeft, topRight, Util.mousePosition)) 
        {
            return true;
        }

        isSelected = false;
        return false;
    }





    SelectablePanel addText(String text) {
        return this;
    }





    SelectablePanel setIsProp(boolean prop) {
        this.isProp = prop;
        return this;
    }





    SelectablePanel setUVSeal(Texture texture) {
        this.uvSeal = texture;
        this.containsUvSeal = true;
        return this;
    }





    public static SelectablePanel create() {
        return null;
    }



    void render(Shader shader, float panelOffset, float yOffset) {
        staticPosition.x = -xSliderOffset;

        Vector2f normalizedScale;
        
        vertexData.vertices = Util.createVertices_CLICKABLEPANE(absoluteSize, new Vector2f(
            (vertexData.localPosition.x + xSliderOffset) / 0.0007f,
            (vertexData.localPosition.y + ySliderOffset) * Util.windowSize.y / (0.0007f * Util.windowSize.x)
        ));

        normalizedScale = Util.createVertexNormalizedScale(absoluteSize, new Vector2f(
            (vertexData.localPosition.x + xSliderOffset) / 0.0007f,
            (vertexData.localPosition.y + ySliderOffset) * Util.windowSize.y / (0.0007f * Util.windowSize.x)
        ));
    }

    void moveTo(Vector2f position, boolean createStatic) {

    }
}
