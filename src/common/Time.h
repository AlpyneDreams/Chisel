#pragma once

#include "common/Common.h"

#include <chrono>

/** Time.h: Controls time for frames and ticks.
 *
 * See: https://gafferongames.com/post/fix_your_timestep/
 */

namespace chisel
{
    inline struct Time
    {
        using Seconds = double;
        using Frames = uint64;

        Seconds time          = 0;      // Scaled time since the dawn of this program
        Seconds deltaTime     = 0;      // Scaled time since last frame finished

        double  timeScale     = 1;      // The speed of time
        Seconds maxDeltaTime  = 0.25;   // The maximum time between frames

        // TODO: Perhaps rename unscaled to realtime or real

        struct Unscaled {               // Real time, unaffected by timeScale or maxDeltaTime
            Seconds time      = 0;      // Real time since the dawn of this program
            Seconds deltaTime = 0;      // Real time since last frame finished
        } unscaled;

        struct Fixed {                  // Fixed update simulation "tick" time
            Seconds time      = 0;      // Time as updated in fixed intervals
            Seconds deltaTime = 0.02;   // Fixed update rate. Default is 50 Hz
        } fixed;

        Frames frameCount     = 0;      // Number of rendered frames
        Frames tickCount      = 0;      // Number of simulated ticks

        Seconds Advance(Seconds dt)
        {
            unscaled.time     += dt;
            unscaled.deltaTime = dt;

            dt *= timeScale;
            time     += dt;
            deltaTime = dt;

            return dt;
        }

        static inline Seconds GetTime()
        {
            using namespace std::chrono;
            return duration<Seconds>(high_resolution_clock::now().time_since_epoch()).count();
        }
    } Time;

}