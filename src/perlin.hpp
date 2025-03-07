#include <vector>
#include <cmath>
#include <algorithm>

namespace app
{

static double smoothstep(double edge_0, double edge_1, double x)
{
    if (edge_0 == edge_1)
    {
        return edge_0;
    }

    double t;
    t = std::clamp((x - edge_0) / (edge_1 - edge_0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

class perlin
{
private:

    struct intersection
    {
        double sin;
        double cos;
    };

    std::vector<intersection>grid;
    uint32_t size_x;
    uint32_t size_y;
    uint32_t m_sample_size_x;
    uint32_t m_sample_size_y;

    double sample(uint32_t grid_x, uint32_t grid_y, double sample_x, double sample_y)
    {
        uint32_t index = grid_y * size_x + grid_x;

        // Vector to intersection
        double ix = sample_x - static_cast<uint32_t>(grid_x);
        double iy = sample_y - static_cast<uint32_t>(grid_y);

        // Dot product
        double dot = grid[index].cos * ix + grid[index].sin * iy;

        return dot;
    }

public:
    perlin(uint32_t x, uint32_t y, uint32_t sample_size_x, uint32_t sample_size_y)
    {
        size_x = x;
        size_y = y;
        m_sample_size_x = sample_size_x;
        m_sample_size_y = sample_size_y;
        grid.resize(static_cast<std::vector<double>::size_type>(x * y));

        for (uint32_t i = 0; i < grid.size(); i++)
        {

            double value = static_cast<double>(std::rand()) * 2.0f * 3.14159f / static_cast<double>(RAND_MAX);
            grid[i].sin = sin(value);
            grid[i].cos = cos(value);
        }
    }

    void set(int x, int y, double value)
    {
        uint32_t index = y * size_x + x;
        grid[index].sin = sin(value);
        grid[index].cos = cos(value);
    }

    double get(double x, double y)
    {
        x *= (size_x - 1);
        y *= (size_y - 1);

        x /= m_sample_size_x;
        y /= m_sample_size_y;

        uint32_t fx = static_cast<uint32_t>(floor(x));
        uint32_t fy = static_cast<uint32_t>(floor(y));
        uint32_t cx = static_cast<uint32_t>(ceil(x));
        uint32_t cy = static_cast<uint32_t>(ceil(y));

        double ul = sample(fx, fy, x, y);
        double ur = sample(cx, fy, x, y);
        double ll = sample(fx, cy, x, y);
        double lr = sample(cx, cy, x, y);

        double weight_x = smoothstep(fx, cx, x);
        double weight_y = smoothstep(fy, cy, y);

        double x1 = std::lerp(ul, ur, weight_x);
        double x2 = std::lerp(ll, lr, weight_x);
        double v = std::lerp(x1, x2, weight_y);

        return v;
    }
};
}
