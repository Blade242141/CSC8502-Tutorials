#include "../NCLGL/window.h"
#include "Renderer.h"
#include <oleidl.h>

int main()	{
	Window w("Make your own project!", 1280, 720, false);

	if(!w.HasInitialised()) {
		return -1;
	}
	
	Renderer renderer(w);
	if(!renderer.HasInitialised()) {
		return -1;
	}

	while(w.UpdateWindow()  && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_K) && Window::GetKeyboard()->KeyTriggered(KEYBOARD_K)) {
			renderer.ToggleAutoCam();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_L) && Window::GetKeyboard()->KeyTriggered(KEYBOARD_L)) {
			renderer.ToggleCamPerspective();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_J) && Window::GetKeyboard()->KeyTriggered(KEYBOARD_J)) {
			renderer.TogglePostProcessing();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_M) && Window::GetKeyboard()->KeyTriggered(KEYBOARD_M)) {
			renderer.ToggleBEV();
		}
	}
	return 0;
}