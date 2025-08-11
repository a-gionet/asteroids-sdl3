#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <time.h>


SDL_Window   *window;
SDL_Renderer *renderer;

_Bool controls[SDL_SCANCODE_COUNT] = {};

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720



struct bullet {
    float x, y;
    float speed_x, speed_y;

    float angle;
};

struct bullet bullets[256];
int bullet_count = 0;

float shot_wait = 0;


struct star {
    float x, y;
    float speed_x, speed_y;

    float scale, angle;
    float grayscale;
};

struct star stars[256] = {};

void create_stars() {
    srand(time(NULL));
    for (int i = 0; i < 256; i ++) {
        stars[i] = (struct star) {
            .x = rand() % WINDOW_WIDTH,
            .y = rand() % WINDOW_HEIGHT,

            .speed_x = 0,
            .speed_y = 15 + i / 4,


            .scale = 3 + i / 32,
            .angle = (float) rand() / 100,
            .grayscale = 0x1A + i / 4,
        };
    }
}


struct player {
    float x, y;
    float speed_x, speed_y;

    float angle;
};

float delta_time = 0;
float propel_time = 0;

void draw_lines(float *angles, float *magnitudes, int point_count, float x, float y, float angle) {
    for (int i = 0; i < point_count; i ++) {
        float x1 = x + magnitudes[i] * cosf(angles[i] + angle);
        float y1 = y + magnitudes[i] * sinf(angles[i] + angle);

        float x2 = x + magnitudes[(i + 1) % 4] * cosf(angles[(i + 1) % point_count] + angle);
        float y2 = y + magnitudes[(i + 1) % 4] * sinf(angles[(i + 1) % point_count] + angle);

        SDL_RenderLine(renderer, x1, y1, x2, y2);
    }
}

void draw_player(struct player *p) {
    float player_angles    [4] = {0, 0.75 * M_PI, 0 , -0.75 * M_PI};
    float player_magnitudes[4] = {0, 24         , 32, 24          };

    draw_lines(player_angles, player_magnitudes, 4, p->x, p->y, p->angle);

    float flame_size = propel_time * (32 + 8 * cosf(30 * (float) SDL_GetTicks() / 1000));

    float fire_angles    [4] = {0, 0.75 * M_PI, M_PI      , -0.75 * M_PI};
    float fire_magnitudes[4] = {0, 16         , flame_size, 16          };
    draw_lines(fire_angles, fire_magnitudes, 4, p->x, p->y, p->angle);
}




void update_player(struct player *p) {
    const float turn_speed = 4;
    if (controls[SDL_SCANCODE_LEFT]) {
        p->angle -= turn_speed * delta_time;
    }
    if (controls[SDL_SCANCODE_RIGHT]) {
        p->angle += turn_speed * delta_time;
    }

    const float propel_speed = 300;
    if (controls[SDL_SCANCODE_UP]) {
        p->speed_x += propel_speed * cosf(p->angle) * delta_time;
        p->speed_y += propel_speed * sinf(p->angle) * delta_time;

		propel_time += 2.5 * delta_time;
		if (propel_time > 1) {
			propel_time = 1;
		}
    } else {
    	propel_time -= 2.5 * delta_time;
    	if (propel_time < 0) {
    		propel_time = 0;
    	}
    }


    p->x += p->speed_x * delta_time;
    p->y += p->speed_y * delta_time;

    if (p->x < -28) {
        p->x = WINDOW_WIDTH + 28;
    }
    if (p->x > WINDOW_WIDTH + 28) {
        p->x = -28;
    }
    if (p->y < -28) {
        p->y = WINDOW_HEIGHT + 28;
    }
    if (p->y > WINDOW_HEIGHT + 28) {
        p->y = -28;
    }
}




int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer("Asteroids!", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);

    
    create_stars();


    struct player p = {
        .x = WINDOW_WIDTH / 2,
        .y = WINDOW_HEIGHT / 2,

        .speed_x = 0,
        .speed_y = 0,

        .angle = 0,
    };


    SDL_Event e;
    while (1) {
        static int last_tick = 0;
        delta_time = ((float) (SDL_GetTicks() - last_tick)) / 1000;
        last_tick = SDL_GetTicks();

        while (SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_EVENT_QUIT:
                goto done;


            case SDL_EVENT_KEY_DOWN:
                controls[e.key.scancode] = true;
                break;

            case SDL_EVENT_KEY_UP:
                controls[e.key.scancode] = false;
                break;
                

            default:
                break;
            }
        }

        
        update_player(&p);

        for (int i = 0; i < 256; i ++) {
            struct star *s = stars + i;
            s->x += s->speed_x * delta_time;
            s->y += s->speed_y * delta_time;
            if (s->y > WINDOW_HEIGHT + s->scale) {
                s->y = -s->scale;
            }
        }


        shot_wait -= delta_time;
        if (shot_wait < 0) {
            shot_wait = 0;
        }
        if (controls[SDL_SCANCODE_SPACE] && shot_wait <= 0 && bullet_count < 256) {
            shot_wait = 0.2;

            bullets[bullet_count ++] = (struct bullet) {
                .x = p.x + 40 * cosf(p.angle),
                .y = p.y + 40 * sinf(p.angle),

                .speed_x = p.speed_x + 400 * cosf(p.angle),
                .speed_y = p.speed_y + 400 * sinf(p.angle),

                .angle = p.angle,
            };
        }


        for (int i = 0; i < bullet_count; i ++) {
            struct bullet *b = bullets + i;


			if (b->x < -28 || b->x > WINDOW_WIDTH + 28 || b->y < -28 || b->y > WINDOW_HEIGHT + 28) {
            	*b = bullets[-- bullet_count];
            	i --; continue;
            }

            
            b->x += b->speed_x * delta_time;
            b->y += b->speed_y * delta_time;

            
        }



        // drawing
        SDL_SetRenderDrawColor(renderer, 0x1A, 0x1A, 0x1A, 0xFF);
        SDL_RenderClear(renderer);

        
        for (int i = 0; i < 256; i ++) {
            struct star *s = stars + i;
            float star_angles[8] = {0, 0.5 * M_PI_2, 1 * M_PI_2, 1.5 * M_PI_2, 2 * M_PI_2, 2.5 * M_PI_2, 3 * M_PI_2, 3.5 * M_PI_2};
            float star_magnitudes[8] = {s->scale, 0.25 * s->scale, s->scale, 0.25 * s->scale, s->scale, 0.25 * s->scale, s->scale, 0.25 * s->scale};
            SDL_SetRenderDrawColor(renderer, s->grayscale, s->grayscale, s->grayscale, 0xFF);
            draw_lines(star_angles, star_magnitudes, 8, s->x, s->y, s->angle);
        }



        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        draw_player(&p);

        if (shot_wait > 0) {
            float x = p.x + 32 * cosf(p.angle);
            float y = p.y + 32 * sinf(p.angle);

            const float angle = M_PI_2 / 3;
            const float size = 18;
            SDL_RenderLine(renderer, 
            	x + size * ((0.2 - shot_wait) / 0.2) * cosf(p.angle + angle), 
                y + size * ((0.2 - shot_wait) / 0.2) * sinf(p.angle + angle),
            	x + size * cosf(p.angle + angle), 
                y + size * sinf(p.angle + angle));
            SDL_RenderLine(renderer, 
               	x + size * ((0.2 - shot_wait) / 0.2) * cosf(p.angle - angle), 
                y + size * ((0.2 - shot_wait) / 0.2) * sinf(p.angle - angle),
               	x + size * cosf(p.angle - angle), 
                y + size * sinf(p.angle - angle));
        }

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        for (int i = 0; i < bullet_count; i ++) {
            struct bullet *b = bullets + i;
            float bullet_angles[2] = {0, M_PI};
            float bullet_magnitudes[2] = {0, 12};
            draw_lines(bullet_angles, bullet_magnitudes, 2, b->x, b->y, b->angle);
        }

        SDL_RenderPresent(renderer);
    }


done:
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
