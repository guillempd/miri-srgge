#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include <glm/gtx/hash.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <limits>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

constexpr float EPS = 1e-3;

struct Ray
{
    glm::vec2 origin;
    glm::vec2 direction;
};

struct RayTraversal
{
    // Current cell
    glm::ivec2 cell;

    // Advance direction along each axis
    int dx, dy;

    // "Time" between intersections of the ray and consecutive coordinate lines of the corresponding axis
    float tx, ty;

    // "Time" left for the next intersection of the ray with a coordinate line of the corresponding axis
    float next_x, next_y;

    // Dimensions of the map
    int w, h;

    RayTraversal(const Ray &ray, int w_, int h_)
    {
        w = w_;
        h = h_;

        cell = glm::ivec2(ray.origin.x, ray.origin.y);
        if (cell.x == w) cell.x = w-1;
        if (cell.y == h) cell.y = h-1;

        tx = std::numeric_limits<float>::max();
        dx = 0;
        if (std::abs(ray.direction.x) > EPS) {
            tx = 1/std::abs(ray.direction.x);
            dx = (ray.direction.x > 0 ? 1 : - 1);
        }

        ty = std::numeric_limits<float>::max();
        dy = 0;
        if (std::abs(ray.direction.y) > EPS) {
            ty = 1/std::abs(ray.direction.y);
            dy = (ray.direction.y > 0 ? 1 : -1);
        }

        next_x = tx;
        // Use similar triangles to determine the correct initial value of next_x (if need to be corrected)
        if (ray.origin.y == 0 || ray.origin.y == h) {
            float fx = frac(ray.origin.x);
            if (ray.direction.x > 0) next_x = (1 - fx) * tx;
            else next_x = fx * tx;
        }

        next_y = ty;
        // Use similar triangles to determine the correct initial value of next_y (if need to be corrected)
        if (ray.origin.x == 0 || ray.origin.x == w) {
            float fy = frac(ray.origin.y);
            if (ray.direction.y > 0) next_y = (1 - fy) * ty;
            else next_y = fy * ty;
        }
    }

    void advance()
    {
        if (next_x < next_y) {
            next_y -= next_x;
            next_x = tx;
            cell.x += dx;
        }
        else { // next_y <= next_x
            next_x -= next_y;
            next_y = ty;
            cell.y += dy;
        }
    }

    bool insideBounds()
    {
        bool inside_x = (0 <= cell.x && cell.x < w);
        bool inside_y = (0 <= cell.y && cell.y < h);
        return inside_x && inside_y;
    }

private:
    static float frac(float x)
    {
        return x - int(x);
    }
};


class VisibilityPrecomputation
{
public:
    VisibilityPrecomputation() = default;

    bool readFloorPlan(const std::string &filename)
    {
        std::string floor_plan_extension = ".tm";
        std::ifstream fin(filename + floor_plan_extension);
        if (!fin.is_open()) return false;

        fin >> w >> h;
        map = std::vector<std::vector<unsigned char>> (w, std::vector<unsigned char>(h));
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                fin >> map[x][y];
            }
        }
        
        visibleFrom = std::vector<std::vector<std::unordered_set<glm::ivec2>>>(w, std::vector<std::unordered_set<glm::ivec2>>(h));
        random_side = std::uniform_int_distribution<int>(0, 2*w + 2*h - 1);
        random_width = std::uniform_real_distribution<float>(0, w);
        random_height = std::uniform_real_distribution<float>(0, h);
        return true;
    }

    bool writeVisibility(const std::string &filename)
    {
        std::string visibility_extension = ".v";
        std::ofstream fout(filename + visibility_extension);
        if (!fout.is_open()) return false;

        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {
                fout << x << ' ' << y;
                for (glm::ivec2 position : visibleFrom[x][y]) {
                    fout << ' ' <<  position.x << ' ' << position.y;
                }
                fout << '\n';
            }
        }
        return true;
    }

    void sampleRays(int n)
    {
        for (int i = 0; i < n; ++i) {
            Ray ray = generateRay();
            traverseRay(ray);
        }
    }


private:
    int randomSide()
    {
        int randomPerimeter = random_side(rng);
        if (randomPerimeter < w) return 0;
        if (randomPerimeter < w + h) return 1;
        if (randomPerimeter < 2*w + h) return 2;
        return 3;
    }

    glm::vec2 randomPoint(int side)
    {
        glm::vec2 point;
        switch (side) {
            case 0:
                point.x = random_width(rng);
                point.y = 0;
                break;
            case 1:
                point.x = 0;
                point.y = random_height(rng);
                break;
            case 2:
                point.x = random_width(rng);
                point.y = h;
                break;
            case 3:
                point.x = w;
                point.y = random_height(rng);
                break;
        }
        return point;
    }

    Ray generateRay()
    {
        int originSide = randomSide();
        glm::vec2 origin = randomPoint(originSide);

        int destinationSide = randomSide();
        while (destinationSide == originSide) destinationSide = randomSide();
        glm::vec2 destination = randomPoint(destinationSide);

        return {origin, glm::normalize(destination - origin)};
    }

    void traverseRay(Ray ray)
    {
        RayTraversal traversal(ray, w, h);
        std::vector<glm::ivec2> currentRoom;
        while(traversal.insideBounds()) {
            unsigned char c = map[traversal.cell.x][traversal.cell.y];
            switch (c) {
                case '.':
                    updateVisibility(traversal.cell, currentRoom, false);
                    break;
                case 'x':
                    currentRoom.clear();
                    break;
                default:
                    currentRoom.push_back(traversal.cell);
                    updateVisibility(traversal.cell, currentRoom, true);
                    break;
            }
            traversal.advance();
        }
    }

    void updateVisibility(glm::ivec2 cell, const std::vector<glm::ivec2> &room, bool newStatue)
    {
        int n = room.size();
        for (int i = 0; i < n; ++i) {
            glm::ivec2 statuePosition = room[i];
            visibleFrom[cell.x][cell.y].insert(statuePosition);
            if (newStatue) visibleFrom[statuePosition.x][statuePosition.y].insert(cell);
        }
    }

private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> random_side;
    std::uniform_real_distribution<float> random_width;
    std::uniform_real_distribution<float> random_height;
    std::vector<std::vector<std::unordered_set<glm::ivec2>>> visibleFrom;
    std::vector<std::vector<unsigned char>> map;
    int w, h;
};

std::string DEFAULT_FILENAME = "test";
constexpr int DEFAULT_N_RAYS = 1e6; 

int main(int argc, char **argv)
{
    std::string filename = DEFAULT_FILENAME;
    if (argc > 1) {
        filename = argv[1];
    }

    int n_rays = DEFAULT_N_RAYS;
    if (argc > 2) {
        n_rays = std::atoi(argv[2]);
    }

    VisibilityPrecomputation vis;
    if (vis.readFloorPlan(filename)) {
        vis.sampleRays(n_rays);
        vis.writeVisibility(filename);
    }
    else std::cerr << "Couldn't load floor plan." << std::endl;
}