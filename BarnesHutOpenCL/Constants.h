#pragma once

// General Constants
static constexpr int NUMBEROFSTARS = 4096;
static constexpr int NUMBEROFGALAXIES = 2;
static constexpr float GALAXYRADIUS = 0.5f;
static constexpr int NUMBEROFNODES = NUMBEROFSTARS * NUMBEROFGALAXIES * 4;

// Compute Constants
static constexpr int BLOCKSIZE = 128;
static constexpr int MAXDEPTH = 32;
static constexpr int WARPSIZE = 32;

// Physics Simulation Constants
static constexpr float PARTICLE_MASS = 1.0e6;
static constexpr float BLACK_HOLE_MASS = 1.0e9;
static constexpr float INITIAL_SPIN_FACTOR = 0.1f;
static constexpr double GRAVITATIONAL_CONSTANT = 6.67408e-11;
static constexpr double SOFTENER = 0.1f;
static constexpr double THETA = 1.0f;