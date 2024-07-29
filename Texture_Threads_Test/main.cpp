#include "Rendererator.h"
#include "../NCLGL/window.h"

int main()
{
	Window w("CSC8599 - Texture Test", 1280, 768, false);

	if (!w.HasInitialised()) {
		return -1;
	}

	Rendererator renderer(w);
	if (!renderer.HasInitialised()) {
		return -1;
	}

	bool showPointer = true;
	w.LockMouseToWindow(showPointer);
	w.ShowOSPointer(!showPointer);
	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE))
	{
		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5))
		{
			Shader::ReloadAllShaders();
		}
	}
}