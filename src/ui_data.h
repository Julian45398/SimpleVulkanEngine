#pragma once

#include "engine_core.h"


inline bool ButtonPressed = false;
inline uint32_t ButtonPressedCount = 0;

void controlUI() {
	static bool show_demo_window = false;
	static shl::Timer timer;
	static int counter;
	double FrameTime = timer.ellapsedMillis();
	ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove);                          // Create a window called "Hello, world!" and append into it.
	ImGui::SetWindowPos(ImVec2(0.f, 0.f));

	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

	//ImGui::SliderFloat("float", &camera.Fov, 10.0f, 120.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
	//ImGui::InputFloat3("Position", (float*)&camera.Pos);
	//ImGui::InputFloat3("Forward", (float*)&camera.Transform[0]);
	//ImGui::InputFloat3("Right", (float*)&camera.Transform[1]);
	//ImGui::InputFloat3("Up", (float*)&camera.Transform[2]);
	//ImGui::Text("yaw: %.3f pitch: %.3f roll: %.3f", camera.Yaw, camera.Pitch, camera.Roll);

	// Buttons return true when clicked (most widgets return true when edited/activated) 
	if (ImGui::Button("Button")) {
		ButtonPressedCount++;
		ButtonPressed = true;
	}
		
	ImGui::SameLine();
	ImGui::Text("counter = %d", ButtonPressedCount);

	ImGui::Text("Application average %.3f ms/frame", FrameTime);
	ImGui::End();
}