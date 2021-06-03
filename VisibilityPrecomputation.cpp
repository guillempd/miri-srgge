#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include <glm/gtx/hash.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <limits>
#include <random>
#include <unordered_set>
#include <vector>

constexpr int N_RAYS = 1e6;
constexpr float EPS = 1e-3;

// GLOBALS
std::vector<std::vector<std::unordered_set<glm::ivec2>>> visibleFrom;
std::vector<std::vector<unsigned char>> map;

std::mt19937 rng;
std::uniform_int_distribution<int> random_side;
std::uniform_real_distribution<float> random_width;
std::uniform_real_distribution<float> random_height;

struct Ray
{
    glm::vec2 origin;
    glm::vec2 direction;
};

struct RayTraversal
{
    glm::ivec2 cell;
    int dx, dy;
    float next_x, next_y;
    float tx, ty;
};

float frac(float x)
{
    return x - int(x);
}

RayTraversal getTraversalFromRay(Ray ray)
{
    int w = map.size();
    int h = map[0].size();

    RayTraversal traversal;

    traversal.tx = std::numeric_limits<float>::max();
    traversal.ty = std::numeric_limits<float>::max();
    traversal.dx = 0;
    traversal.dy = 0;

    if (std::abs(ray.direction.x) > EPS) {
        traversal.tx = 1/std::abs(ray.direction.x);
        traversal.dx = (ray.direction.x > 0 ? 1 : - 1);
    }
    if (std::abs(ray.direction.y) > EPS) {
        traversal.ty = 1/std::abs(ray.direction.y);
        traversal.dy = (ray.direction.y > 0 ? 1 : -1);
    }

    traversal.cell = glm::ivec2(ray.origin.x, ray.origin.y);
    if (traversal.cell.x == w) traversal.cell.x = w-1;
    else if (traversal.cell.y == h) traversal.cell.y = w-1;

    traversal.next_x = traversal.tx;
    traversal.next_y = traversal.ty;

    if (ray.origin.x == 0 || ray.origin.x == w) {
        if (ray.direction.y > 0) traversal.next_y = (1 - frac(ray.origin.y)) * traversal.ty;
        else traversal.next_y = frac(ray.origin.y) * traversal.ty;
    }

    else if (ray.origin.y == 0 || ray.origin.y == h-1) {
        if (ray.direction.x > 0) traversal.next_y = (1 - frac(ray.origin.x)) * traversal.tx;
        else traversal.next_x = frac(ray.origin.x) * traversal.tx;
    }

    return traversal;
}

int randomSide()
{
    int w = map.size();
    int h = map[0].size();
    int randomPerimeter = random_side(rng);
    if (randomPerimeter < w) return 0;
    if (randomPerimeter < w + h) return 1;
    if (randomPerimeter < 2*w + h) return 2;
    return 3;

}

glm::vec2 randomPoint(int side)
{
    int w = map.size();
    int h = map[0].size();
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
    while (originSide == destinationSide) destinationSide = randomSide();
    glm::vec2 destination = randomPoint(destinationSide);

    return {origin, glm::normalize(destination - origin)};
}

void updateVisibility(const std::vector<glm::ivec2> &room)
{
    int n = room.size();
    glm::ivec2 newStatuePosition = room.back();
    for (int i = 0; i < n-1; ++i) {
        glm::ivec2 statuePosition = room[i];
        visibleFrom[statuePosition.x][statuePosition.y].insert(newStatuePosition);
        visibleFrom[newStatuePosition.x][newStatuePosition.y].insert(statuePosition);
    }
}

void advance(RayTraversal &traversal)
{
    if (traversal.next_x < traversal.next_y) {
        traversal.next_x = traversal.tx;
        traversal.next_y -= traversal.next_x;
        traversal.cell.x += traversal.dx;
    }
    else { // traversal.next_y <= traversal.next_x
        traversal.next_y = traversal.ty;
        traversal.next_x -= traversal.next_y;
        traversal.cell.y += traversal.dy;
    }
}

bool insideMap(const RayTraversal &traversal)
{
    int w = map.size();
    int h = map[0].size();
    bool inside_x = (0 <= traversal.cell.x && traversal.cell.x < w);
    bool inside_y = (0 <= traversal.cell.y && traversal.cell.y < h);
    return inside_x && inside_y;
}

void traverseRay(Ray ray)
{
    RayTraversal traversal = getTraversalFromRay(ray);
    std::vector<glm::ivec2> currentRoom;
    while(insideMap(traversal)) {
        unsigned char c = map[traversal.cell.x][traversal.cell.y];
        switch(c) {
            case '.':
                break;
            case 'x':
                currentRoom.clear();
                break;
            default:
                currentRoom.push_back(traversal.cell);
                updateVisibility(currentRoom);
                break;
        }
        advance(traversal);
    }
}

void readMap()
{
    std::ifstream fin("visibility.in");
    if (!fin.is_open()) std::exit(EXIT_FAILURE);

    int w, h;
    fin >> w >> h;
    map = std::vector<std::vector<unsigned char>> (w, std::vector<unsigned char>(h));
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            fin >> map[x][y];
        }
    }
}

void initialize()
{
    int w = map.size();
    int h = map[0].size();
    visibleFrom = std::vector<std::vector<std::unordered_set<glm::ivec2>>>(w, std::vector<std::unordered_set<glm::ivec2>>(h));
    random_side = std::uniform_int_distribution<int>(0, 2*w + 2*h - 1);
    random_width = std::uniform_real_distribution<float>(0, w);
    random_height = std::uniform_real_distribution<float>(0, h);
}

void writeVisibility()
{
    std::ofstream fout("visibility.out");
    if (!fout.is_open()) std::exit(EXIT_FAILURE);

    int w = map.size();
    int h = map[0].size();
    fout << w * h << '\n';
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            fout << x << ' ' << y;
            for (glm::ivec2 position : visibleFrom[x][y]) {
                fout << ' ' <<  position.x << ' ' << position.y;
            }
            fout << '\n';
        }
    }
}

int main()
{
    readMap();
    initialize();
    for (int i = 0; i < N_RAYS; ++i) {
        Ray ray = generateRay();
        traverseRay(ray);
    }
    writeVisibility();
}