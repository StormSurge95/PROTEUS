#pragma once

#include "../shared/NES_PCH.h"
#include "./Pulse.h"
#include "./Triangle.h"
#include "./Noise.h"
#include "./DMC.h"

namespace NES_NS {
    class APU : IDevice<u8, u16> {
            // Allow Debugger class to access all private members of the APU class
            friend class Debugger;
        public:
            /**
             * @brief Reference to the BUS object so we can more easily perform DMCDMA
             */
            wptr<CPU> cpu;

            /**
             * @brief Flag for whether or not an IRQ has been requested by the APU.
             */
            bool irqRequested = false;

            /**
             * @brief Explicit Constructor
             * @details
             * Creates unique_ptr objects for each channel.
             */
            APU();

            /**
             * @brief Default Destructor
             */
            ~APU() = default;

            /**
             * @brief Connects the bus to the APU.
             * @param b 
             */
            inline void connectCPU(sptr<CPU> c) { cpu = c; }

            /**
             * @brief Read data from APU registers.
             * @param addr Address to read.
             * @param readonly Flag to block side-effects.
             * @return The data that was read; or open bus if invalid address.
             */
            u8 read(u16 addr, bool readonly = false) override;

            /**
             * @brief Write data to APU registers.
             * @param addr Address to be written to.
             * @param data Data to be written.
             */
            void write(u16 addr, u8 data) override;

            /**
             * @brief Cycle function for the APU.
             * @details
             * One full APU cycle is equivalent to two CPU cycles
             * and is composed of a GET cycle and a PUT cycle.
             * 
             * We are operating our APU on each CPU cycle and
             * separating the various operations based on which
             * half of the APU sequence we are on.
             * 
             * Every other cycle (GET <OR> PUT):
             *      Clocks Pulse channels
             * 
             * Every cycle (GET <AND> PUT):
             *      Clocks Triangle channel
             *      Clocks Noise channel
             *      Clocks Frame counter
             *      Generates new audio sample.
             */
            void clock();

            /**
             * @brief Provides all produces audio samples via modification of pass-by-reference argument.
             * @param [in,out] buffer Reference to master sample buffer used for audio playback
             * @details
             * Adds the contents of internal sample buffer
             * to the buffer referenced by `buffer`
             * then clears the internal sample buffer.
             */
            void collectSamples(vector<float>& buffer);

            /// @brief get the current sample address from DMC
            u16 getDmcCurrentAddr() const { return dmc->getCurrAddr(); }
            /// @brief get the base sample address from DMC
            u16 getDmcSampleAddr() const { return dmc->getSampleAddr(); }
            /** @brief set the new sample byte within DMC
             *  @param data the value to push to DMC sample buffer
             */
            void setDmcSampleByte(u8 data) { dmc->setSampleByte(data); }

            /**
             * @brief Handles call to DMC channel sample fetch method.
             * @param first Whether or not this is the first call to `dmcFetch` for this round.
             */
            inline void dmcFetch(bool first) { dmc->fetchSample(first); }

        private:
            /// @brief Used to keep track of when to reset FrameCounter.
            u64 masterCycle = 0x0000000000000000;
            /// @brief Current APU cycle.
            u16 cycle = 0x0000;
            /// @brief Target `masterCycle` to reset FrameCounter at.
            u64 resetAt = 0x0000000000000000;
            /// @brief The "open bus" value of the APU.
            u8 apuBus = 0x00;
            /// @brief Flag to trigger FrameCounter cycle reset.
            bool pendingReset = false;

            /// @brief Flag for which FrameCounter sequence to use.
            bool use5step = false;
            /// @brief Flag for inhibiting IRQ
            bool inhibitIRQ = false;

            /// @brief The audio sample rate of the APU
            /// @todo Implement some way of making the audio dynamic based on the console region.
            const double CYCLES_PER_SAMPLE = 1789773.0 / 44100.0;
            /// @brief Accumulator variable for producing audio samples.
            double cycleAccumulator = 0.0;
            /// @brief Buffer to hold produced audio samples
            vector<float> sampleBuffer;

            /// @brief First square wave channel
            uptr<PulseChannel> pulse1 = nullptr;
            /// @brief Second square wave channel
            uptr<PulseChannel> pulse2 = nullptr;
            /// @brief Triangle wave channel
            uptr<TriangleChannel> triangle = nullptr;
            /// @brief Digial noise channel
            uptr<NoiseChannel> noise = nullptr;
            /// @brief Delta modulation channel
            uptr<DMC_Channel> dmc = nullptr;

            HighPassFilter HPF1, HPF2;
            LowPassFilter LPF;

            /**
             * @brief Performs necessary logic for producing half/quarter/end frame clock signals.
             */
            void clockFrameCounter();

            /**
             * @brief Performs necessary operations for quarter-frame clock signals.
             */
            void quarterFrame();
            /**
             * @brief Performs necessary operations for half-frame clock signals.
             */
            void halfFrame();

            /**
             * @brief Generates a completed audio sample and places it into the buffer.
             */
            void generateSample();

            /**
             * @brief Helper function for reading the APU status from $4015
             * @return The current status of the APU and it's channels.
             */
            u8 read4015();
            /**
             * @brief Helper function for writing to APU Control at $4015
             * @param data The data to be written.
             */
            void write4015(u8 data);
            /**
             * @brief Helper function for writing to APU FrameCounter at $4017
             * @param data The data to be written.
             */
            void write4017(u8 data);

            /**
             * @brief Mixes the various channels audio samples into one value to be placed in the buffer
             * @param pulse1 square 1 sample
             * @param pulse2 square 2 sample
             * @param triangle triangle sample
             * @param noise noise sample
             * @param dmc dmc sample
             * @return The fully mixed audio sample.
             */
            float mixSamples(u8 pulse1, u8 pulse2, u8 triangle, u8 noise, u8 dmc);
    };
}