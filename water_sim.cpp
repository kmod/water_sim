#include <GL/glut.h> // GLUT, include glu.h and gl.h
#include <list>
#include <sys/time.h>
#include <unistd.h>
#include <cstdio>
#include <cmath>

struct Drop {
    float x, y;
    float vx, vy;
};

struct Wall {
    float x, y;
    float r;
};

std::list<Drop> drops;
std::list<Wall> walls;

#define R 0.02f // radius
#define DT 0.03 // tick increment per frame
#define FR 60   // framerate
#define G -1    // gravity strength (& direction)

void drawCircle(float x, float y, float r) {
    //glBegin(GL_LINE_LOOP);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 12; i++) {
        float theta = 2 * 3.14159265 * i / 12;
        glVertex2f(x + r * cosf(theta), y + r * sinf(theta));
    }
    glEnd();
}

/* Handler for window-repaint event. Call back when the window first appears and
   whenever the window needs to be re-painted. */
void display() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
    glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double w = glutGet(GLUT_WINDOW_WIDTH);
    double h = glutGet(GLUT_WINDOW_HEIGHT);
    double ar = w / h;
    glOrtho(-2 * ar, 2 * ar, -2, 2, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    glColor3f(0.5f, 0.6f, 0.3f);
    for (const auto& w : walls) {
        glBegin(GL_LINE_LOOP);
        glVertex2f(w.x - w.r, w.y - w.r); // x, y
        glVertex2f(w.x + w.r, w.y - w.r);
        glVertex2f(w.x + w.r, w.y + w.r);
        glVertex2f(w.x - w.r, w.y + w.r);
        glEnd();
    }

    glColor3f(0.2f, 0.3f, 1.0f);
    for (const auto& d : drops) {
        drawCircle(d.x, d.y, R);
        //glVertex2f(d.x - R, d.y - R); // x, y
        //glVertex2f(d.x + R, d.y - R);
        //glVertex2f(d.x + R, d.y + R);
        //glVertex2f(d.x - R, d.y + R);
    }

    glFlush(); // Render now
}

#define MAX_INTERACT R

void tick() {
    static struct timeval last_idle_time;
    struct timeval time_now;
    gettimeofday(&time_now, NULL);

    float dt = (float)(time_now.tv_sec - last_idle_time.tv_sec) + 1.0e-6 * (time_now.tv_usec - last_idle_time.tv_usec);
    if (dt < 1.0 / FR) {
        usleep(1000000 * (1.0 / FR - dt));
        tick();
        return;
    }
    gettimeofday(&last_idle_time, NULL);

    float energy = 0;

    for (auto& d : drops) {
        energy += 0.5 * (d.vx * d.vx + d.vy * d.vy) - G * d.y;

        // printf("(%.1f, %.1f) going (%.1f, %.1f)\n", d.x, d.y, d.vx, d.vy);
        d.vy += DT * G / 2;
        d.x += DT * d.vx;
        d.y += DT * d.vy;

        for (auto& w : walls) {
            /*if (fabsf(d.x - w.x) < R + w.r) {
                if (d.vy > 0 && d.y < w.y && w.y - d.y < R + w.r) {
                    printf("cw1\n");
                    d.vy = -d.vy;
                }
                if (d.vy < 0 && d.y > w.y && d.y - w.y < R + w.r) {
                    printf("cw2\n");
                    d.vy = -d.vy;
                }
            }
            if (fabsf(d.y - w.y) < R + w.r) {
                if (d.vx > 0 && d.x < w.x && w.x - d.x < R + w.r) {
                    printf("cw3\n");
                    d.vx = -d.vx;
                }
                if (d.vx < 0 && d.x > w.x && d.x - w.x < R + w.r) {
                    printf("cw4\n");
                    d.vx = -d.vx;
                }
            }*/
            float dx = d.x - w.x;
            float dy = d.y - w.y;

            if (fabs(dx) > R + w.r || fabs(dy) > R + w.r)
                continue;
            if (fabs(dx) > w.r && fabs(dy) > w.r)
                continue;

            float dvx = d.vx;
            float dvy = d.vy;
            float fdx = fabs(dx);
            float fdy = fabs(dy);

            if (dx > 0 && dx > fdy && dvx < 0) {
                //printf("cw1\n");
                d.vx = -d.vx * 0.6;
                d.x += DT * DT * (R + w.r - dx) * 500;
            } else if (dx < 0 && -dx > fdy && dvx > 0) {
                //printf("cw2\n");
                d.vx = -d.vx * 0.6;
                d.x -= DT * DT * (R + w.r + dx) * 500;
            } else if (dy > 0 && dy > fdx && dvy < 0) {
                //printf("cw3\n");
                d.vy = -d.vy * 0.6;
                d.vy -= G * DT; // hack to make the drops not fall through walls
                d.y += DT * DT * (R + w.r - dy) * 500;
            } else if (dy < 0 && -dy > fdx && dvy > 0) {
                //printf("cw4\n");
                d.vy = -d.vy * 0.6;
                d.vy -= G * DT; // hack to make the drops not fall through walls
                d.y -= DT * DT * (R + w.r + dy) * 500;
            } else {
                // printf("cw ???\n");
            }
        }
        d.vy += DT * G / 2;

        for (auto& d2 : drops) {
            if (d2.x == d.x && d2.y == d.y)
                continue;

            float dx = d.x - d2.x;
            float dy = d.y - d2.y;

            //if (fabs(dx) > R + R + MAX_INTERACT || fabs(dy) > R + R + MAX_INTERACT)
                //continue;

            float dsq = dx * dx + dy * dy;
            if (dsq > 6 * R * R)
                continue;

            //printf("(%f %f) (%f %f)\n", d.x, d.y, d2.x, d2.y);

//#define RIGIDITY (1 / R / R / R / R)
#define RIGIDITY (2 / R / R)
#define RX 0.1f
#define TENSION 0.00005
#define FRICTION 0.01

            float dvx, dvy;
            if (dsq < 4 * R * R) {
                float mag = 4 * R * R - dsq;
                //mag = mag * mag * RIGIDITY + RX;
                mag = mag * RIGIDITY + RX;
                dvx = (d.x - d2.x) * mag;
                dvy = (d.y - d2.y) * mag;
            } else {
                //float mag = 4 * R * R - dsq;
                float mag = -1.0 / dsq;
                //d.vx *= 0.999;
                //d.vy *= 0.999;
                //d2.vx *= 0.999;
                //d2.vy *= 0.999;
                dvx = (d.x - d2.x) * mag * TENSION;
                dvy = (d.y - d2.y) * mag * TENSION;
            }

            dvx += FRICTION * (d2.vx - d.vx);
            dvy += FRICTION * (d2.vy - d.vy);

            d.vx += dvx;
            d2.vx -= dvx;
            d.vy += dvy;
            d2.vy -= dvy;

#if 0
            float dvx = 0;
            float dvy = 0;

#define TENSION 0.03
#define RIGIDITY 100
            if (dx > R + R || dx < -R - R) {
                //dvx = -TENSION * R * dx / (R * R * R * R + (dx * dx) * (dx * dx));
            } else if (dx > 0) {
                printf("1\n");
                dvx = (R + R - dx) * RIGIDITY;
            } else {
                printf("2\n");
                dvx = (-R - R - dx) * RIGIDITY;
            }
            if (dy > R + R || dy < -R - R) {
                //dvy = -TENSION * R * dy / (R * R * R * R + (dy * dy) * (dy * dy));
            } else if (dy > 0) {
                printf("3\n");
                dvy = (R + R - dy) * RIGIDITY;
            } else {
                printf("4\n");
                dvy = (-R - R - dy) * RIGIDITY;
            }
            dvx *= DT;
            dvy *= DT;
            printf("%f %f\n", dvx, dvy);

            if (1) {
                float f1 = d.vx - d2.vx;
                d.vx += dvx;
                d2.vx -= dvx;
                float f2 = d.vx - d2.vx;
                printf("vx: %f %f\n", f1, f2);
            }
            if (1) {
                float f1 = d.vy - d2.vy;
                d.vy += dvy;
                d2.vy -= dvy;
                float f2 = d.vy - d2.vy;
                printf("vy: %f %f\n", f1, f2);
            }
#endif
        }
    }

    printf("System energy: %.2f\n", energy);
    glutPostRedisplay();
}

/* Main function: GLUT runs as a console application starting at main()  */
int main(int argc, char** argv) {
    glutInit(&argc, argv);                 // Initialize GLUT
    glutCreateWindow("OpenGL Setup Test"); // Create a window with the given title
    glutInitWindowSize(320, 320);          // Set the window's initial width & height
    glutInitWindowPosition(50, 50);        // Position the window's initial top-left corner
    glutDisplayFunc(display);              // Register display callback handler for window re-paint
    glutIdleFunc(tick);

#if 0
    //drops.push_back(Drop({.x = 0, .y = 0 }));
    //drops.push_back(Drop({.x = 0.2, .y = 0 }));
    for (int i = 0; i < 2; i++) {
        drops.push_back(Drop({.x = 0.35f * (i - 2), .y = 0.5, .vx = -0.1f * (i - 4), .vy = 0.3 }));
    }
    //drops.push_back(Drop({.x = 0.08, .y = 1 }));
#else
    //drops.push_back(Drop({.x = 0, .y = 0 }));
    //drops.push_back(Drop({.x = 0.2, .y = 0 }));
    int nx = 42;
    int ny = 42;
    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
            drops.push_back(Drop({.x = R * 2.01f * i - 1.2f, .y = 0.2f + R * 2.01f * j, .vx = 1.5f, .vy = -0.8 }));
        }
    }
    //drops.push_back(Drop({.x = 0.08, .y = 1 }));
#endif

    static const float W = 1.5f;
    walls.push_back(Wall({.x = .6f, .y = -0.2, .r = 0.3}));
    for (float x = -W; x <= W + 0.001; x += 0.25) {
        walls.push_back(Wall({.x = x, .y = -1, .r = 0.25 }));
    }
    for (float y = -1 + 0.25; y <= 1.001; y += 0.25) {
        walls.push_back(Wall({.x = -W, .y = y, .r = 0.25 }));
        walls.push_back(Wall({.x = W, .y = y, .r = 0.25 }));
    }

    glutMainLoop(); // Enter the event-processing loop
    return 0;
}
