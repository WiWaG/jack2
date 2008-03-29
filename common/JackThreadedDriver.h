/*
 Copyright (C) 2001 Paul Davis
 Copyright (C) 2004-2008 Grame

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

#ifndef __JackThreadedDriver__
#define __JackThreadedDriver__

#include "JackDriver.h"
#include "JackThread.h"

namespace Jack
{

/*!
\brief The base class for threaded drivers. Threaded drivers are used with blocking devices.
*/

class JackThreadedDriver : public JackDriverClientInterface, public JackRunnableInterface
{

    private:

        JackThread* fThread;
        JackDriverClient* fDriver;

    public:

        JackThreadedDriver(JackDriverClient* driver);
        virtual ~JackThreadedDriver();

        virtual int Open()
        {
            return fDriver->Open();
        }

        virtual int Open(jack_nframes_t nframes,
                         jack_nframes_t samplerate,
                         bool capturing,
                         bool playing,
                         int inchannels,
                         int outchannels,
                         bool monitor,
                         const char* capture_driver_name,
                         const char* playback_driver_name,
                         jack_nframes_t capture_latency,
                         jack_nframes_t playback_latency)
        {
            return fDriver->Open(nframes, samplerate, capturing, playing, inchannels, outchannels, monitor, capture_driver_name, playback_driver_name, capture_latency, playback_latency);
        }

        int Close()
        {
            return fDriver->Close();
        }

        int Process()
        {
            return fDriver->Process();
        }

        int Attach()
        {
            return fDriver->Attach();
        }
        int Detach()
        {
            return fDriver->Detach();
        }

        int Read()
        {
            return fDriver->Read();
        }
        int Write()
        {
            return fDriver->Write();
        }

        int Start();
        int Stop();

        int SetBufferSize(jack_nframes_t buffer_size)
        {
            return fDriver->SetBufferSize(buffer_size);
        }

        int SetSampleRate(jack_nframes_t sample_rate)
        {
            return fDriver->SetSampleRate(sample_rate);
        }

        void SetMaster(bool onoff)
        {
            fDriver->SetMaster(onoff);
        }
        bool GetMaster()
        {
            return fDriver->GetMaster();
        }

        void AddSlave(JackDriverInterface* slave)
        {
            fDriver->AddSlave(slave);
        }
        void RemoveSlave(JackDriverInterface* slave)
        {
            fDriver->RemoveSlave(slave);
        }
        int ProcessSlaves()
        {
            return fDriver->ProcessSlaves();
        }

        int ClientNotify(int refnum, const char* name, int notify, int sync, int value1, int value2)
        {
            return fDriver->ClientNotify(refnum, name, notify, sync, value1, value2);
        }

        JackClientControl* GetClientControl() const
        {
            return fDriver->GetClientControl();
        }

        bool IsRealTime()
        {
            return fDriver->IsRealTime();
        }

        // JackRunnableInterface interface
        bool Execute();
        bool Init();

};

} // end of namespace


#endif
