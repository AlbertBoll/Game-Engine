#pragma once
#include"Codes.h"

class Window;


void InputBackend_Init(Window& window);
void InputBackend_Shutdown();

// Polling queries used by Input::BeginFrame()
bool InputBackend_GetKeyDown(Key k);
bool InputBackend_GetMouseDown(MouseButton b);
void InputBackend_GetMousePos(float& x, float& y);