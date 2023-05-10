#include"board.h"
int main(int argc, char* argv[])
{
	srand(time(0));
	titanium::window app("ShonetsuSIX");
	ShonetsuSIX::board board;
	using namespace titanium;

	while (app.is_running())
	{
		board.draw(app);
		app.render();
	}
	return 0;
}