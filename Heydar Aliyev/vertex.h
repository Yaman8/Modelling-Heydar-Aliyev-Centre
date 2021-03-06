#pragma once

#include "Coord.h"
#include "colorOptions.h"

class Vertex
{
public:
    vect4 pos;
    vect4 normal;
    vect4 color;

public:
    Vertex() = default;

    Vertex(const vect4& pos, const vect4& normal, const vect4& c): color(c),normal(normal),pos(pos){}

    Vertex& operator+=(const Vertex& rhs)
    {
        pos = pos + rhs.pos;
        color = color + rhs.color;
        return *this;
    }
    Vertex operator+(const Vertex& rhs) const
    {
        return Vertex(*this) += rhs;
    }
    Vertex& operator-=(const Vertex& rhs)
    {
        pos = pos - rhs.pos;
        color = color - rhs.color;
        return *this;
    }
    Vertex operator-(const Vertex& rhs) const
    {
        return Vertex(*this) -= rhs;
    }
    Vertex& operator*=(float rhs)
    {
        pos = pos * rhs;
        color = color * rhs;
        return *this;
    }
    Vertex operator*(float rhs) const
    {
        return Vertex(*this) *= rhs;
    }
    Vertex& operator/=(float rhs)
    {
        pos = pos / rhs;
        color = color / rhs;
        return *this;
    }
    Vertex operator/(float rhs) const
    {
        return Vertex(*this) /= rhs;
    }
};