#pragma once

// Compute Constants
static constexpr int BLOCKSIZE = 256;
static constexpr int MAXDEPTH = 32;
static constexpr int WARPSIZE = 32;

// Physics Simulation Constants
static constexpr float PARTICLE_MASS = 1.0e6;
static constexpr float BLACK_HOLE_MASS = 1.0e8;
static constexpr float INITIAL_SPIN_FACTOR = 0.0f;
static constexpr double GRAVITATIONAL_CONSTANT = 6.67408e-11;
static constexpr double SOFTENER = 0.1;
static constexpr double THETA = 1.0;