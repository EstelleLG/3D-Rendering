#pragma once

#define GLM_FORCE_RADIANS 
#define M_PI    3.14159265358979323846264338327950288
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\quaternion.hpp"
#include "glm\gtc\random.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "glm\gtx\simd_mat4.hpp"
#include "glm\gtx\simd_quat.hpp"
#include "glm\gtx\simd_vec4.hpp"
#include "glm\gtx\matrix_decompose.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "resource.h"
#include "stdafx.h"
#include "GL/glew.h"
#include "GL/wglew.h"
#include "Commctrl.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#define BERRYBLEND_WINDOWS32 
namespace Berryblend {
#ifdef BERRYBLEND_WINDOWS32


	class System {
		HINSTANCE applicationInstance;
		friend bool InitApi(HINSTANCE);
	public:
		System() {

		}
		System(const System&) = delete;
		System& operator = (const System&) = delete;

		HINSTANCE GetInstance() const{
			return applicationInstance;
		}

	};


	class Brush {
		HBRUSH hbrush;
	public:
		Brush(int color) {
			hbrush = CreateSolidBrush(color);
		}

		~Brush() {
			DeleteObject(hbrush);
		}

		HBRUSH GetBrush() const{
			return hbrush;
		}

	};

	class Surface {
		HDC deviceContext;
	public:
		Surface(const HDC& hdc) {
			deviceContext = hdc;
		}
		Surface(const Surface&) = delete;
		Surface& operator = (const Surface&) = delete;

		HDC GetHDC() const {
			return deviceContext;
		}

		void DrawText(std::wstring str, int bottom, int right, int style) {
			RECT rect;
			rect.bottom = bottom;
			rect.left = 0;
			rect.top = 0;
			rect.right = right;
			::DrawText(deviceContext, str.c_str(), str.length(), &rect, style);
		}

		void DrawRect(int x, int y, int width, int height) {
		
			::Rectangle(deviceContext, x, y, x + width, y + height);
		}

		void SetBrush(const Brush& brush) {
			SelectObject(deviceContext, brush.GetBrush());
		}

		void SwapBuffers() {
			::SwapBuffers(deviceContext);
		}


	};

	

	class Window {
		HWND window;
		
	public :
		Window(const System& system, std::wstring classname, std::wstring caption, int x, int y, int width, int height, uint32_t style);
		Window(const Window&) = delete;   //can't copy
		Window& operator = (const Window&) = delete;

		Surface* GetSurface() {
			return new Surface(GetDC(window));
		}

		void ReleaseSurface(Surface* surface) {
			ReleaseDC(window, surface->GetHDC());
			delete surface;
		}

		HWND GetHWND() {
			return window;
		}

		void showWindow() {
			ShowWindow(window, SW_SHOW);
		}


		class PaintSurface {
			PAINTSTRUCT ps;
			Surface* surface;
			Window* window;
		public:
			PaintSurface(Window* window) : window(window){
				HDC hdc = ::BeginPaint(window->GetHWND(), &ps);
				surface = new Surface(hdc);
				
			}
			~PaintSurface() {
				::EndPaint(this->window->GetHWND(), &ps);
				delete surface;
			}

			Surface* GetSurface() {
				return surface;
			}
			
		};


		PaintSurface* BeginPaint() {
			return new PaintSurface(this);
		}
		void EndPaint(PaintSurface* ps) {
			delete ps;
		}

		virtual void Paint(Surface& surface) {
			

			surface.DrawText(L"NO CREATIVITY", 1000, 500, 0);

			Brush brush(RGB(30, 40, 255));
			surface.SetBrush(brush);
			surface.DrawRect(100, 100, 500, 500);
			
		}


	};

	


	LRESULT(CALLBACK SUBCLASSPROC)(
		HWND      hWnd,
		UINT      uMsg,
		WPARAM    wParam,
		LPARAM    lParam,
		UINT_PTR  uIdSubclass,
		DWORD_PTR dwRefData
		) {
		switch (uMsg)
		{
		case WM_PAINT:
		{
			Window* window = (Window*)dwRefData;
			auto ps = window->BeginPaint();

			window->Paint(*(ps->GetSurface()));
			window->EndPaint(ps);
			
		}
		break;
		case WM_CLOSE:
		{
			auto msg = MessageBox(hWnd, L"Dont kill me!!", L"HAHAHAH", MB_OKCANCEL);
			if (msg == IDOK) PostQuitMessage(0);

		}
		break;
		default:
			return DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}
		return 0;
	}

	class GLContext {
		HGLRC context;
		HDC deviceContext;
		PIXELFORMATDESCRIPTOR dc_pfd;
		GLuint dc_pixel_format;
	public:
		GLContext(const Surface& surface) {

			PIXELFORMATDESCRIPTOR pfd =
			{
				sizeof(PIXELFORMATDESCRIPTOR),
				1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
				PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
				32,                        //Colordepth of the framebuffer.
				0, 0, 0, 0, 0, 0,
				0,
				0,
				0,
				0, 0, 0, 0,
				24,                        //Number of bits for the depthbuffer
				8,                        //Number of bits for the stencilbuffer 
				0,                        //Number of Aux buffers in the framebuffer.
				PFD_MAIN_PLANE,
				0,
				0, 0, 0
			};

			HDC glHdc = surface.GetHDC();

			GLuint pixelformat;

			if (!(pixelformat = ChoosePixelFormat(glHdc, &pfd)))
			{
				printf("Error choosing pixelformat!\n");
				return;
			}

			if (SetPixelFormat(glHdc, pixelformat, &pfd) != TRUE)
			{
				printf("Error setting pixelformat!\n");
				return;
			}

			dc_pfd = pfd;
			dc_pixel_format = pixelformat;

			HGLRC temp;

			if (!(temp = wglCreateContext(glHdc)))
			{
				printf("Error creating dummy context\n");
				return;
			}

			if (!wglMakeCurrent(glHdc, temp))
			{
				printf("Error binding glContext\n");
				return;
			}

			glewExperimental = GL_TRUE;

			GLenum glewError = glewInit();
			if (glewError != GLEW_OK)
			{
				printf("Error initializing GLEW %s\n", glewGetErrorString(glewError));
				return;
			}

			int attribs[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				WGL_CONTEXT_MINOR_VERSION_ARB, 3,
				WGL_CONTEXT_FLAGS_ARB,
				WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
				0
			};

			context = wglCreateContextAttribsARB(glHdc, NULL, attribs);

			if (context == NULL)
			{
				printf("Error creating final RenderContext\n");
				return;
			}

			wglSwapIntervalEXT(0);
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(temp);
			wglMakeCurrent(glHdc, context);
		}
		
		void Clear(int bits) {
			glClear(bits);
		}

		void SetClearColor(float r, float g, float b, float a) {
			glClearColor(r, g, b, a);
		}

	};

	extern System currentSystem;


	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

	bool InitApi(HINSTANCE applicationInstance) {

		currentSystem.applicationInstance = applicationInstance;

		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = applicationInstance;
		wcex.hIcon = LoadIcon(applicationInstance, MAKEINTRESOURCE(IDI_BERRYBLEND));
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = 0;
		wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_BERRYBLEND);
		wcex.lpszClassName = L"MinuteMaid";
		wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
		
		if (!RegisterClassExW(&wcex)) {
			return false;
		}


		auto m_Console = AllocConsole();

		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);

		return true;
	}

	class GLShader {
		GLenum type;
		GLuint glShader;
	public:
		GLShader(GLenum type) : type(type) {
			glShader = glCreateShader(type);
		}

		bool loadAndCompile(std::string filename) {
			std::ifstream file(filename);
			std::string shaderString;
			if (file.is_open()) {
				std::string temp;
				while (std::getline(file, temp)) {
					shaderString += temp;
					shaderString += '\n';
				}
			}

			auto str = shaderString.c_str();
			GLint strLen = shaderString.length();
			glShaderSource(glShader, 1, &str, &strLen);
			glCompileShader(glShader);
			
			GLint error;
			glGetShaderiv(glShader, GL_COMPILE_STATUS, &error);

			return error != GL_FALSE;

		}

		std::string getErrorString() {
			
			GLint len;
			glGetShaderiv(glShader, GL_INFO_LOG_LENGTH, &len);
			std::string buffer(len, 'x');
			glGetShaderInfoLog(glShader, len, &len, &buffer[0]);
			return buffer;
		}

		GLuint getID() const{
			return glShader;
		}

	};


	class StopWatch {
		double PCFreq = 0.0;
		int64_t CounterStart = 0;
	public:
		StopWatch()
		{
			LARGE_INTEGER li;
			if (!QueryPerformanceFrequency(&li))
			{
				return;
			}

			PCFreq = double(li.QuadPart) / 1000.0;

			QueryPerformanceCounter(&li);
			CounterStart = li.QuadPart;
		}

		inline void Begin()
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			CounterStart = li.QuadPart;
		}

		inline double Restart()
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			auto ret = double(li.QuadPart - CounterStart) / PCFreq;
			CounterStart = li.QuadPart;
			return ret;
		}

		inline double GetTimeNow() const
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			return double(li.QuadPart - CounterStart) / PCFreq;
		}

		inline double End()
		{
			auto time = GetTimeNow();
			CounterStart = 0;
			return time;
		}
	};
	
	class GLProgram {
		GLuint glprogram;
	public:
		GLProgram() {
			glprogram = glCreateProgram();
		}
		GLuint getGLProgram() {
			return glprogram;
		}

		void attachShader(const GLShader& shader) {
			glAttachShader(glprogram, shader.getID());
		}

		bool linkProgram() {
			glLinkProgram(glprogram);

			int error;
			glGetProgramiv(glprogram, GL_LINK_STATUS, &error);
			if (error == GL_FALSE) {
				return false;
			}
			return true;
		}

		std::string getErrorString() {
			int length;
			glGetProgramiv(glprogram, GL_INFO_LOG_LENGTH, &length);

			std::string buffer(length, 'w');
			glGetProgramInfoLog(glprogram, length, &length, &buffer[0]);
			return buffer;

		}

		int getUniformLocation(const std::string& name) {
			return glGetUniformLocation(glprogram, name.c_str());
		}

		void useProgram() {
			glUseProgram(glprogram);
		}

	};

	class GLArrayBuffer {
		GLuint glarraybuffer;
	public:
		GLArrayBuffer() {
			glGenBuffers(1, &glarraybuffer);
		}

		void bindBuffer() {
			glBindBuffer(GL_ARRAY_BUFFER, glarraybuffer);
		}

		void bufferData(unsigned int size, void* data, GLenum usage) {
			glBufferData(GL_ARRAY_BUFFER, size, data, usage);
		}
	};

	class GLElementBuffer {
		GLuint glelementbuffer;
	public:
		GLElementBuffer() {
			glGenBuffers(1, &glelementbuffer);
		}

		void bindBuffer() {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glelementbuffer);
		}

		void bufferData(unsigned int size, void* data, GLenum usage) {
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
		}
	};

	class GLVertexArray {
		GLuint vao;
	public:
		GLVertexArray() {
			glGenVertexArrays(1, &vao);
		}
		void bindVertexArray() {
			glBindVertexArray(vao);
		}

		void unbindVertexArray() {
			glBindVertexArray(0);
		}

		void vertexAttribPointer(unsigned int vertexLocation, unsigned int numComponents, GLenum type, bool normalized,  unsigned int stride, void* offset) {
			glEnableVertexAttribArray(vertexLocation);
			glVertexAttribPointer(vertexLocation, numComponents, type, normalized ? GL_TRUE : GL_FALSE, stride, offset);
		}

		GLuint getGLId() {
			return vao;
		}

		
	};

	class GLTexture2D {
		GLuint gltexture2d;

	public:
		GLTexture2D() {
			glGenTextures(1, &gltexture2d);
		}
		GLuint getGLId(){
			return gltexture2d;
		}
		void bindTexture() {
			glBindTexture(GL_TEXTURE_2D, gltexture2d);
		}
		void loadData(GLuint internalFormal, unsigned int width, unsigned int height, GLuint dataFormat, GLenum type, void* data) {
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormal, width, height, 0, dataFormat, type, data);
			
		}
		void setParameter(GLint minParam, GLint magParam) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minParam);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magParam);
		}
		void setWrapST(GLenum S, GLenum T) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, S);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, T);
		}
		void setCompareFcn(GLenum fcn) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, fcn);
		}
		void setCompareMode(GLenum mode) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, mode);
		}
	};

	struct Vertex {
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		Vertex() {

		}
		Vertex(float x, float y, float z) : position(x, y, z){
			
			
		}
		Vertex(glm::vec3 position, glm::vec2 uv, glm::vec3 normal) : position(position), uv(uv), normal(normal) {
			
		}
	};

	inline Vertex operator* (const glm::mat4& mat, const Vertex& v) {
		auto ret = v;
		ret.position = glm::vec3(mat * glm::vec4(ret.position, 1));
		ret.normal = glm::vec3(mat * glm::vec4(ret.normal, 0));
		return ret;
	}

	class GLWindow : public Window {
	protected: GLContext* glc;
	public:
		GLWindow(const System& system, std::wstring classname, std::wstring caption, int x, int y, int width, int height, uint32_t style) 
			: Window(system, classname, caption, x, y, width, height, style){
			
			glc = new GLContext(*(this->GetSurface()));

			
			
		}

	};

	

#endif
}
