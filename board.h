#pragma once
#include<vector>
#include"titanium/window.h"
#include<SDL_timer.h>
#include<random>
#include<array>
#define tile_size 64
#define tile_at(x) x * tile_size
#define tile_sub(x) x -= tile_size
#define tile_add(x) x += tile_size
namespace ShonetsuSIX
{
	namespace assets
	{
		titanium::font bismark_font("bismark");
		titanium::texture chess{ "sprite.png" };
		titanium::texture pawn{ "white statue.png" };
	}

	struct tile :titanium::object
	{
		enum segment
		{
			right,
			up,
			left,
			down
		};

		segment my_segment;
		int index;
		titanium::text label;
		tile* back{ nullptr };

		tile(object* parent, titanium::v2<> new_position, segment new_flow, int new_index, tile* back_ptr = nullptr) :
			object({ 0,0 }, { 6.f,6.f }),
			my_segment(new_flow),
			index(new_index),
			label(assets::bismark_font, { 0,0 }),
			back(back_ptr)
		{

			this->set_texture(assets::chess);
			this->set_parent(parent);

			label.set_pivot({ .5f, .5f });
			label.set_text(std::to_string(index));
			label.set_parent(this);

			if (back != nullptr)
			{
				titanium::v2<float> pivot_segment;
				if (my_segment == tile::right)
				{
					pivot_segment = { 1.f,0.f };
				}
				else if (my_segment == tile::down)
				{
					pivot_segment = { 0.f,1.f };
				}
				else if (my_segment == tile::left)
				{
					pivot_segment = { -1.f,0.f };
				}
				else if (my_segment == tile::up)
				{
					pivot_segment = { 0.f,-1.f };
				}
				place_at(*back, pivot_segment);
			}
		}
	};

	class player : public titanium::object
	{
		size_t _queue{ 0 };
		size_t _position_on_board{ 1 };
	public:
		player(object* parent) :
			object({ 0,0 }, { 2.0f,2.0f }, { .5f,.5f })
		{
			set_parent(parent);
		}
		void roll()
		{
			_queue = (rand() % 6) + 1;
			_position_on_board += _queue;
		}
		size_t get_position_on_board()
		{
			return _position_on_board;
		}
		void step() {
			_queue--;
		}
		size_t get_step()
		{
			return _position_on_board - (_queue);
		}
		void reset()
		{
			_queue = 0;
			_position_on_board = 1;
		}
		bool in_progress() const
		{
			return _queue > 0;
		}
	};

	class board :titanium::object
	{
		std::vector<std::shared_ptr<tile>> _path;
		std::vector<std::shared_ptr<player>> _players;
		size_t _length{ 12 * 8 };

		void new_path()
		{
			_path.reserve(_length);
			_path.clear();
			_path.emplace_back(new tile(this, titanium::v2{ tile_at(7), tile_at(5) }, tile::right, 1));
			std::array < size_t, 12> flow_ar{ 1, 0, 1, 2, 1, 2, 3, 2, 3, 0, 3 };
			for (size_t i = 1; i < _length; i++)
			{
				tile* back = _path[i - 1].get();
				titanium::v2 new_position{ back->get_position() };

				_path.emplace_back(new tile(this, new_position, static_cast<tile::segment>(flow_ar[i / 8]), i + 1, back));
			}
		}

		titanium::text score{ assets::bismark_font,{0,0} };

	public:
		board(titanium::window app)
		{
			app.load_texture(assets::pawn);
			app.load_texture(assets::chess);
			_players.emplace_back(new player(this));// create player

			new_path();
			_players[0]->set_position(_path[0]->get_absolute_position());
			_players[0]->set_texture(assets::pawn);
			_players[0]->place_at(*_path[0], { .5f,.5f });

		}
		void draw(titanium::window& app)
		{

			static int path_position{ 1 };

			if (app.key_pressed(SDL_SCANCODE_F5))
				new_path();
			if (app.key_pressed(SDL_SCANCODE_SPACE) && !_players[0]->in_progress())
				_players[0]->roll();
			if (app.key_pressed(SDL_SCANCODE_LEFT))
				move(64, 0);
			else if (app.key_pressed(SDL_SCANCODE_RIGHT))
				move(-64, 0);
			if (app.key_pressed(SDL_SCANCODE_DOWN))
				move(0, -64);
			else if (app.key_pressed(SDL_SCANCODE_UP))
				move(0, 64);
			if (app.key_pressed(SDL_SCANCODE_SPACE))
			{
				int dice_result{ (rand() % 6) + 1 };
				path_position += 1;//dice_result;
				if (path_position > _length)
					path_position = 1;
			}
			int counter = 1;
			for (auto t : _path)
			{
				app.draw_texture(*t);
				app.draw_text(t->label);
				counter++;
			}

			for (auto player : _players)
			{
				static size_t round = 0;
				static float time = 0.f;
				static titanium::v2 now;
				static titanium::v2 next;

				//run player role
				if (player->in_progress())
				{
					if (time >= 1.f)
					{
						if (round >= _length - 1)
						{
							player->reset();
						}
						else
						{
							player->step();
						}
						time = 0;
						round = player->get_step() - 1;
						std::stringstream formated;

						std::ostringstream formated_score;
						formated_score << "score: " << round;
						score.set_text(formated_score.str());
					}
					else
					{
						player->place_at(*_path[round >= _length - 1 ? 0 : round + 1], { .5f,.5f }, _path[round]->get_absolute_position(), time);
						time += abs(sin(SDL_GetTicks() / 160)) * 0.0080f;
					}
				}
				app.draw_texture(*player);

			}
			app.draw_text(score);
		}
	};
}