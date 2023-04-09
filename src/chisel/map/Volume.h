#pragma once

namespace chisel
{
    namespace Volumes
    {
        enum Volume
        {
            Auto, Air, Solid, Window
        };
    }
    
    using Volume = Volumes::Volume;
}
