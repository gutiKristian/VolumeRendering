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

		/*
		 * Application data initialization
		*/
		void OnStart();

		/*
		 * Updating data
		 */
		void OnUpdate(base::Timestep ts);

		/*
		 * Render calls.
		 */
		void OnRender();

		/*
		 * Rendering UI.
		 */
		void OnImGuiRender();

		/*
		 * Called when application is closed (not in ems for now)
		 */
		void OnEnd();
	private:
		/*
		 * Main loop of the application
		 */
		void Run();

		/*
		 * Event processing stuff and pre-precessing things around window
		 */
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
