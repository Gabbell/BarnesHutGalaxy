#pragma once

// Compute Constants
static constexpr int BLOCKSIZE = 512;
static constexpr int MAXDEPTH = 32;
static constexpr int WARPSIZE = 32;

// Physics Simulation Constants
static constexpr float PARTICLE_MASS = 1.0e5;
static constexpr float BLACK_HOLE_MASS = 1.0e9;
static constexpr float INITIAL_SPIN_FACTOR = 0.3f;
static constexpr double GRAVITATIONAL_CONSTANT = 6.67408e-11;
static constexpr float SOFTENER = 0.1f;
static constexpr float THETA = 0.1f;