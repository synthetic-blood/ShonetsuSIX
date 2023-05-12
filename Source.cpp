#include"board.h"
int main(int argc, char* argv[])
{
	srand(time(0));
	titanium::window app("ShonetsuSIX");
	ShonetsuSIX::board board(app);

	app.load_texture(ShonetsuSIX::assets::pawn);

	while (app.is_running())
	{
		board.draw(app);
		app.render();
	}
	SDL_Quit();
	return 0;
}