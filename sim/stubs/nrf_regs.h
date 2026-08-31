#pragma once
#include <cstdint>
// The display layer pokes SPIM registers directly on hardware; here they are
// just memory, because nothing downstream of them is being simulated.
struct SimSPIM { uint32_t ENABLE; struct { uint32_t SCK, MOSI, MISO; } PSEL; };
extern SimSPIM sim_spim0, sim_spim1, sim_spim2, sim_spim3;
#define NRF_SPIM0 (&sim_spim0)
#define NRF_SPIM1 (&sim_spim1)
#define NRF_SPIM2 (&sim_spim2)
#define NRF_SPIM3 (&sim_spim3)
