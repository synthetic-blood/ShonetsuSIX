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
	}

	struct tile :titanium::object
	{
		enum flow
		{
			right,
			up,
			left,
			down
		};

		flow my_flow;
		int index;
		titanium::text label;
		tile* back{ nullptr };

		tile(object* parent, titanium::v2 new_position, flow new_flow, int new_index, tile* back_ptr = nullptr) :
			object(new_position, { 64,64 }),
			my_flow(new_flow),
			index(new_index),
			label(assets::bismark_font, { 0,0 }),
			back(back_ptr)
		{
			if (index != 0)
				this->set_parent(parent);
			label.set_parent(this);
			label.set_text(std::to_string(index));
		}
	};

	class player : public titanium::object
	{
		size_t _queue{ 0 };
		size_t _position_on_board{ 1 };
	public:
		player(object* parent) :
			object({ 0,0 }, { tile_size / 2,tile_size / 2 })
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
		size_t _length{ 12 * 4 };
		const titanium::v2 tile_center{ tile_size / 4,tile_size / 4 };
		void new_path()
		{
			_path.reserve(_length);
			_path.clear();
			_path.emplace_back(new tile(this, titanium::v2{ tile_at(7), tile_at(5) }, tile::right, 1));
			std::array < size_t, 12> flow_ar{ 1, 0, 1, 2, 1, 2, 3, 2, 3, 0, 3 };
			for (size_t i = 1; i < _length; i++)
			{
				tile* back = _path[i - 1].get();
				titanium::v2 new_position = back->get_position();

				tile::flow new_dir{ static_cast<tile::flow>(flow_ar[i / 4]) };

				if (new_dir == tile::right)
				{
					tile_add(new_position.x);
				}
				else if (new_dir == tile::down)
				{
					tile_add(new_position.y);
				}
				else if (new_dir == tile::left)
				{
					tile_sub(new_position.x);
				}
				else if (new_dir == tile::up)
				{
					tile_sub(new_position.y);
				}

				_path.emplace_back(new tile(this, new_position, new_dir, i + 1, back));
			}
		}
	public:
		board()
		{
			new_path();

			_players.emplace_back(new player(this));// create player
			_players[0]->set_position(_path[0]->get_absolute_position() + tile_center);
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
				Uint8 shade = abs(sin((SDL_GetTicks() / 512.f) * counter * 0.01f)) * 127 + 32;
				const bool is_target = _players[0]->in_progress() && _players[0]->get_position_on_board() == counter;

				SDL_Color color{ shade, shade - (is_target ? 32 : 0),shade - (is_target ? 32 : 0) };
				app.draw_rect(*t, color);
				app.draw_text(t->label);
				counter++;
			}
			for (auto player : _players)
			{
				static size_t round = 0;
				static float time = 0.f;
				static titanium::v2 now;
				static titanium::v2 next;
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

					}
					else
						time += abs(cos(SDL_GetTicks() / 80)) * 0.0080f;

					titanium::v2 now = _path[round]->get_absolute_position();
					titanium::v2 next = _path[round >= _length - 1 ? 0 : round + 1]->get_absolute_position();

					player->set_position(titanium::v2{
						(int)std::lerp(now.x,
							next.x, time),
						 (int)std::lerp(now.y,
							next.y, time) } + tile_center);
				}
				app.draw_rect(*player, { 0,0,0 });
			}
		}
	};
}