#pragma once

#include "Base/Window.h"
#include "Base/Timestep.h"

int main(int argc, char* argv[]);

namespace med {

	class Application
	{
	public:
		Application();
		~Application();

		void OnUpdate(base::Timestep ts);
		void OnRender();
		void OnImGuiRender();
	private:
		void Run();
		void OnFrame(base::Timestep ts);
	private:
#if defined(PLATFORM_WEB)
		static int EMSRedraw(double time, void* userData);
#endif
	private:
		base::Window* m_Window;
		bool m_Running = false;
	private:
		friend int ::main(int argc, char* argv[]);
	};

}
