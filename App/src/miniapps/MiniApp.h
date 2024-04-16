namespace med
{
	class MiniApp
	{
	public:
		virtual void OnStart() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
		virtual void OnEnd() = 0;
		virtual void OnImGuiRender() = 0;
	};
}