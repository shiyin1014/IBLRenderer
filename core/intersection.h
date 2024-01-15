//
// Created by shiyin on 2023/12/20.
//
#pragma once

#ifndef SIMPLERENDERER_INTERSECTION_H
#define SIMPLERENDERER_INTERSECTION_H

class Model;

struct Intersection{

    Intersection(){
        happened = false;
        model = nullptr;
        distance = std::numeric_limits<float>::max();
    }

    bool happened;
    double distance;
    Model* model;
};

#endif //SIMPLERENDERER_INTERSECTION_H
