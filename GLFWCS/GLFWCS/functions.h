#pragma once
#include <iostream>
#include <windows.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include "VectorDT.h"



void ShowMenu(GLFWwindow* window)
{
	std::cout << "hiding menu\n";
	glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, true);
}


void HideMenu(GLFWwindow* window)
{
	std::cout << "showing menu\n";
	glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, false);
}

void ConvertToRange(Vec3& Point)
{
	Point.X /= 2560.0f;
	Point.X *= 2.0f;
	Point.X -= 1.0f;

	Point.Y /= 1440.0f;
	Point.Y *= 2.0f;
	Point.Y -= 1.0f;
}

bool WorldToScreen(const Vec3& VecOrigin, Vec3& VecScreen, float* Matrix, int height, int width)
{
	VecScreen.X = VecOrigin.X * Matrix[0] + VecOrigin.Y * Matrix[1] + VecOrigin.Z * Matrix[2] + Matrix[3];
	VecScreen.Y = VecOrigin.X * Matrix[4] + VecOrigin.Y * Matrix[5] + VecOrigin.Z * Matrix[6] + Matrix[7];
	VecScreen.Z = VecOrigin.X * Matrix[8] + VecOrigin.Y * Matrix[9] + VecOrigin.Z * Matrix[10] + Matrix[11];
	float W = VecOrigin.X * Matrix[12] + VecOrigin.Y * Matrix[13] + VecOrigin.Z * Matrix[14] + Matrix[15];

	if (W < 0.01f)
		return false;

	Vec2 NDC;
	NDC.X = VecScreen.X / W;
	NDC.Y = VecScreen.Y / W;

	VecScreen.X = (2560 / 2 * NDC.X) + (NDC.X + 2560 / 2);
	VecScreen.Y = (1440 / 2 * NDC.Y) + (NDC.Y + 1440 / 2);

	ConvertToRange(VecScreen);

	return true;
}

void DrawLine(Vec2& Start, Vec3& End, ColorA* Color)
{
	glBegin(GL_LINES);
	glColor4f(Color->R, Color->G, Color->B, Color->A);
	glVertex2f(Start.X, Start.Y);
	glVertex2f(End.X, End.Y);
	glEnd();
}

float GetDistance(Vec3 to, Vec3 from)
{
	float deltaX = to.X - from.X;
	float deltaY = to.Y - from.Y;
	float deltaZ = to.Z - from.Z;
	return (float)sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
}