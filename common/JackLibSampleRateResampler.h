/*
Copyright (C) 2008 Grame

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef __JackLibSampleRateResampler__
#define __JackLibSampleRateResampler__

#include "JackResampler.h"
#include <samplerate.h>

namespace Jack
{

inline float Range(float min, float max, float val)
{
    return (val < min) ? min : ((val > max) ? max : val);
}
    
/*!
\brief Resampler using "libsamplerate" (http://www.mega-nerd.com/SRC/).
*/

class JackLibSampleRateResampler : public JackResampler
{

    private:
    
        SRC_STATE* fResampler;
        double fRatio;
           
    public:
    
        JackLibSampleRateResampler();
        JackLibSampleRateResampler(unsigned int quality, unsigned int ringbuffer_size);
        virtual ~JackLibSampleRateResampler();
        
        unsigned int ReadResample(float* buffer, unsigned int frames);
        unsigned int WriteResample(float* buffer, unsigned int frames);
         
        void SetRatio(unsigned int num, unsigned int denom)
        {
            JackResampler::SetRatio(num, denom);
            fRatio = Range(0.25f, 4.0f, (double(num) / double(denom)));
        }
        
        void Reset();
          
    };
}

#endif
