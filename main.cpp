#include <iostream>
#include <SFML/Graphics.hpp>
#include "solver.hpp"
#include "renderer.hpp"

static sf::Color rgb(float t) {
	const float r = sin(t);
	const float g = sin(t + .33f * 2.f * 3.14159f);
	const float b = sin(t + .66f * 2.f * 3.14159f);
	return { static_cast<uint8_t>(255.0f * r * r),
		static_cast<uint8_t>(255.0f * g * g),
		static_cast<uint8_t>(255.0f * b * b) };
}

int main() {
	constexpr int window_width = 1000;
	constexpr int window_height = 1000;

	sf::ContextSettings settings;
	settings.antialiasingLevel = 1;
	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Verlet Sim", sf::Style::Default, settings);
	int framerate = 60;
	window.setFramerateLimit(framerate);

	Solver solver;
	Renderer renderer(window);

	// Sim attributes
	const float obj_spawn_delay = .2f;
	const float obj_spawn_speed = 750.f;
	const sf::Vector2f obj_spawn_pos = { 100.f, 100.f };
	const float obj_min_rad = 5.f;
	const float obj_max_rad = 20.f;
	const int max_objs = 5000;
	const float max_angle = 1.f;

	// Config for solver
	// Window Constraint
	solver.setConstraint({ window_width, window_height });
	solver.setSubSteps(8);
	solver.setSimUpdateRate(framerate);

	sf::Clock clock;
	
	// Game Loop
	while (window.isOpen()) {
		sf::Event event{};
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
				window.close();
			}
		}

		if (/*solver.getObjCount() < max_objs && */ clock.getElapsedTime().asSeconds() < obj_spawn_delay) {
			clock.restart();
			const float t = solver.getTime();

			auto& o = solver.addObject(obj_spawn_pos, obj_min_rad);
			solver.setObjVel(o, { obj_spawn_speed, 0.f });
			o.col = rgb(t);

			
			auto& o2 = solver.addObject({ obj_spawn_pos.x, obj_spawn_pos.y + 20 }, obj_min_rad);
			solver.setObjVel(o2, { obj_spawn_speed, 0.f });
			o2.col = rgb(t);

			auto& o3 = solver.addObject({ obj_spawn_pos.x, obj_spawn_pos.y + 40 }, obj_min_rad);
			solver.setObjVel(o3, { obj_spawn_speed, 0.f });
			o3.col = rgb(t);
			
			auto& o4 = solver.addObject({ obj_spawn_pos.x, obj_spawn_pos.y + 60 }, obj_min_rad);
			solver.setObjVel(o4, { obj_spawn_speed, 0.f });
			o4.col = rgb(t);

			auto& o5 = solver.addObject({ obj_spawn_pos.x, obj_spawn_pos.y + 80 }, obj_min_rad);
			solver.setObjVel(o5, { obj_spawn_speed, 0.f });
			o5.col = rgb(t);
			

			std::cout << "Objects: " << solver.getObjCount() << "\n";
		}

		// Options: 
		// -updateNaive();
		// -updateOptim();
		// -updateThrea();
		solver.updateOptim();
		window.clear(sf::Color::Black);
		renderer.render(solver);
		window.display();
	}
}
