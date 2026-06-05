#include "../PAK/Mappers/NesMapper.h"
#include "./NesPPU.h"

using namespace NS_NES;

void PPU::powerup(u32 s) {
    initPRNG(s);

    // initialize timing position
    scanline = 0;
    cycle = 0;
    oddFrame = false;
    frameComplete = false;

    // clear transient NMI/render flags
    nmiRequested = false;
    nmiOutput = false;
    suppressNMI = false;
    suppressVBL = false;

    // initialize register/latch state(s)
    PPUCTRL = 0;
    PPUMASK = 0;
    PPUSTATUS = 0xA0 | (nextByte() & ~0xA0);
    OAMADDR = 0;
    OAMDATA = 0;
    PPUSCROLL = 0;
    PPUADDR = 0;
    PPUDATA = 0;
    OAMDMA = 0;
    dataBuffer = 0;
    w = false;
    t = v = x = 0;
    
    // initialize open-bus helpers
    ppuBus = 0;
    ppuAddrBus = 0;

    // initialize volatile memory containers
    for (u8& byte : nametables) byte = nextByte();
    for (u8& byte : palettes) byte = nextByte();
    for (array<u8, 4>& row : primaryOAM) {
        for (u8& byte : row) byte = nextByte();
    }
    for (array<u8, 5>& row : secondaryOAM) {
        for (u8& byte : row) byte = nextByte();
    }
    // initialize decay counters
    for (u8& c : decayCounters) c = 20;

    // initialize background/sprite pipelines
    clearPipelines();

    ignoreEarlyCtrlWrites = true;
}

void PPU::reset() {
    // reset timing position
    scanline = cycle = 0;
    oddFrame = frameComplete = false;

    // clear transient NMI/render flags
    nmiRequested = nmiOutput = suppressNMI = suppressVBL = false;

    // clear registers affected by reset
    PPUCTRL = PPUMASK = PPUSCROLL = dataBuffer = 0;
    w = false;
    t = x = 0;

    clearPipelines();

    ignoreEarlyCtrlWrites = true;
}

void PPU::powerdown() {
    // stop further nmi generation
    nmiRequested = nmiOutput = suppressNMI = suppressVBL = false;

    // clear timing/transient exec state
    scanline = cycle = 0;
    frameComplete = oddFrame = false;

    // clear transient bus/scratch state
    ppuAddrBus = ppuBus = dataBuffer = 0;

    clearPipelines();

    ignoreEarlyCtrlWrites = false;
}

void PPU::clearPipelines() {
    // clear background pipeline state
    nextNametableByte = nextAttributeByte = nextPatternByteLo = nextPatternByteHi = 0;
    bgPatternAddr = patternShiftLo = patternShiftHi = attributeShiftLo = attributeShiftHi = 0;

    // clear sprite pipeline state
    spriteIndex = n = m = oamLatch = spritesOnScanline = oamIndex = byteIndex = 0;
    sprTileIndex = sprAttributes = sprXPosition = sprPatternLo = sprPatternHi = 0;
    spritePatternAddr = 0;
    sprite0HitOnNextScanline = sprite0HitOnThisScanline = false;
    activeSprites.fill({});
}

/** via https://nesdev.org/wiki/NMI#Operation
 * Two 1-bit registers inside the PPU control the generation of NMI signals. Frame timing and accesses to the PPU's
 * PPUCTRL and PPUSTATUS registers change these registers as follows, regardless of whether rendering is enabled:
 *      1. Start of vertical blanking (scanline 241, dot 1): Set vblank_flag in PPU to true.
 *      2. End of vertical blanking (scanline 261, dot 1): Set vblank_flag to false.
 *      3. Read PPUSTATUS: Return old status of vblank_flag in bit 7, then set vblank_flag to false.
 *      4. Write to PPUCTRL: Set NMI_output to the state of bit 7.
 * 
 * If 1 and 3 happen simultaneously, PPUSTATUS bit 7 is read as false, and vblank_flag is set to false anyway.
 * This means that the NMI won't fire, and that the program will be unaware that the hardware is in VBLANK.
 * 
 * The PPU pulls NMI low IF AND ONLY IF both vblank_flag and NMI_output are true.
 */
u8 PPU::read(u16 addr, bool readonly) {
    u8 ret = ppuBus;
    if (addr >= 0x2000 && addr <= 0x3FFF) {
        // mask address due to mirroring
        addr &= 0x0007;
        switch (addr) {
            case 0x02: // read from PPUSTATUS register
                ret = (PPUSTATUS & 0xE0) | (ppuBus & 0x1F);
                if (!readonly) {
                    updateCounters(0xE0);
                    if (scanline == 241) {
                        if (cycle <= 1) {
                            suppressVBL = true;
                        } else if (cycle == 2) {
                            suppressNMI = true;
                        }
                    }
                    inVBlank(false);
                    w = false;
                    recompNMI();
                }
                break;
            case 0x04: // read from OAMDATA
                // set our return value to the byte within OAM1
                ret = readOAMByte(OAMADDR); // reads do not increment OAMADDR
                if (readonly) return ret;
                // update counters for bit decay
                updateCounters(0xFF);
                break;
            case 0x07: // read from PPUDATA
                {
                    u16 addr = v & 0x3FFF;
                    u8 data = ppuRead(addr, readonly);

                    if (addr >= 0x3F00) {
                        ret = data;
                        if (readonly) return ret;
                        dataBuffer = ppuRead(addr - 0x1000);
                    } else {
                        ret = dataBuffer;
                        if (readonly) return ret;
                        dataBuffer = data;
                    }
                    v = (v + getVRAMIncrement()) & 0x3FFF;
                    ppuBus = ret;
                    updateCounters(0xFF);
                }
                break;
        }
    }
    // by this point, ppubus should have been updated using the requested value;
    // so we can simply return ppubus at this point (also this prevents the
    // "not all control paths return a value" issue within VS)
    ppuBus = ret;
    return ppuBus;
}

void PPU::write(u16 addr, u8 data) {
    if (addr >= 0x2000 && addr <= 0x3FFF) {
        // update ppuBus with new data
        ppuBus = data;
        // update counters for bit decay
        updateCounters(0xFF);
        // mask address for mirroring
        addr &= 0x0007;

        switch (addr) {
            case 0x00: // write to PPUCTRL
                if (!ignoreEarlyCtrlWrites) {
                    // update register
                    PPUCTRL = data;
                    // update t register bits 11 & 12 using bits 0 & 1 of new data
                    t = ((t & 0xF3FF) | ((u16)(data & 0x03) << 10));
                    recompNMI();
                    return;
                }
            case 0x01: // write to PPUMASK
                if (!ignoreEarlyCtrlWrites) PPUMASK = data;
                return;
            case 0x03: // write to OAMADDR
                OAMADDR = data;
                return;
            case 0x04: // write to OAMDATA
                writeOAMByte(OAMADDR, data); // update OAM
                OAMADDR++; // increment address
                return;
            case 0x05: // write to PPUSCROLL
                if (!ignoreEarlyCtrlWrites) {
                    PPUSCROLL = data;
                    if (!w) { // first write to PPUSCROLL consists of X-scroll bits 7-0 (we got bit 8 from PPUCTRL)
                        // update 'x' to be the right 3 bits of 'data'
                        x = (data & 0x07);

                        // update 't' using left 5 bits of 'data'
                        t &= ~0x001F; // clear the right 5 bits of 't'
                        t |= ((u16)(data >> 3) & 0x1F); // set them to the value of the left 5 bits of 'data'

                        // set write latch so that next write to register will work properly
                        w = true;
                    } else { // second write to PPUSCROLL consists of Y-scroll bits 7-0 (we got bit 8 from PPUCTRL)
                        // FEDC BA98 7654 3210 = 76543210
                        // .LMN ..GH IJK. ....   GHIJKLMN
                        
                        // clear bits E-C & 9-5
                        t &= 0x0C1F;

                        // update bits E-C of 't' using bits 2-0 of 'data'
                        t |= ((u16)(data & 0x07) << 12);
                        // update bits 9-5 of 't' using bits 7-3 of 'data'
                        t |= ((u16)(data & 0xF8) << 2);

                        // clear write latch so that next write to register will work properly
                        w = false;
                    }
                }
                return;
            case 0x06: // write to PPUADDR
                if (!ignoreEarlyCtrlWrites) {
                    // writes to PPUADDR update the VRAM address
                    if (!w) { // first write update bits 13-8 using t
                        // clear previous high byte of 't'
                        t &= 0x00FF;
                        // update 't' with new high byte
                        // we use a mask because 't' only has 15 bits and bit 15 is explicitly cleared anyways
                        t |= ((u16)(data & 0x3F) << 8);
                        // set write latch so that next write works properly
                        w = true;
                    } else { // second write update bits 7-0 using t and copies final value to v
                        // clear previous low byte of 't'
                        t &= 0x7F00;
                        // update 't' with new low byte (here we simply use all supplied bits)
                        t |= ((u16)data);
                        // transfer 't' to 'v'
                        // theoretically, the mask here is unnecessary; but better safe than sorry.
                        v = t & 0x3FFF;
                        // clear write latch so next write works properly
                        w = false;
                    }
                }
                return;
            case 0x07: // write to PPUDATA
                PPUDATA = data; // update register
                ppuWrite(v, PPUDATA); // update vram using value
                v = (v + getVRAMIncrement()) & 0x3FFF; // increment vram address
                return;
        }
    }
}

u8 PPU::ppuRead(u16 addr, bool readonly) {
    if (!readonly) ppuAddrBus = addr;
    addr &= 0x3FFF; // mask address because ppu memory map only goes up to 0x3FFF
    u8 ret = 0x00; // temp var for return value
    
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        // if address is within CHR memory, read from gamepak/mapper
        ret = cart.lock()->mapper->ppuRead(addr, readonly);
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        // if address is within nametable memory, get nametable byte
        addr &= 0x0FFF; // mask address for use as array index
        u16 A = addr & 0x03FF; // nametable at $2000
        u16 B = A + 0x0400;    // nametable at $2400
        u16 C = B + 0x0400;    // nametable at $2800
        u16 D = C + 0x0400;    // nametable at $2C00

        /* via https://www.nesdev.org/wiki/PPU_nametables#Mirroring
         * nametable layout looks as follows:
         *      (0,0)     (256,0)     (511,0)
         *        +-----------+-----------+
         *        |           |           |
         *        |    NT1    |    NT2    |
         *        |   $2000   |   $2400   |
         *        |           |           |
         *        |           |           |
         * (0,240)+-----------+-----------+(511,240)
         *        |           |           |
         *        |    NT3    |    NT4    |
         *        |   $2800   |   $2C00   |
         *        |           |           |
         *        |           |           |
         *        +-----------+-----------+
         *      (0,479)   (256,479)   (511,479)
         */
        switch (cart.lock()->getMirror()) {
            default:
            case MIRROR::VERTICAL: // horizontal arrangement
                // vertical mirror means NT1 = NT3 and NT2 = NT4
                // update return value to relevant nametable based on address
                if (addr <= 0x03FF) ret = nametables[A];
                else if (addr <= 0x07FF) ret = nametables[B];
                else if (addr <= 0x0BFF) ret = nametables[A];
                else ret = nametables[B];
                break;
            case MIRROR::ONE_SCREEN_LO:
                // "one screen lo" (also "one screen a") means all four tables are NT1
                ret = nametables[A];
                break;
            case MIRROR::ONE_SCREEN_HI:
                // "one screen hi" (also "one screen b") means all four tables are NT2
                ret = nametables[B];
                break;
            case MIRROR::HORIZONTAL: // vertical arrangement
                // horizontal mirror means NT1 = NT2 and NT3 = NT4
                // update return value to relevant nametable based on address
                if (addr <= 0x03FF) ret = nametables[A];
                else if (addr <= 0x07FF) ret = nametables[A];
                else if (addr <= 0x0BFF) ret = nametables[B];
                else ret = nametables[B];
                break;
            case MIRROR::FOUR_SCREEN:
                // four screen means none of the nametables are identical
                // update return value to relevant nametable based on address
                if (addr <= 0x03FF) ret = nametables[A];
                else if (addr <= 0x07FF) ret = nametables[B];
                else if (addr <= 0x0BFF) ret = nametables[C];
                else ret = nametables[D];
                break;
        }
    } else {
        // lastly, if address is within palette memory, get palette byte
        addr &= 0x1F; // mask address for use as array index
        // these four addresses are simply mirrors of index 0
        // any other addresses lead to unique master palette indexes
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;

        // palette ram bytes are only 6 bits wide, causing a range of 0-63
        // so that the byte can be used as an index into the 64-byte array
        // of master palette color values; however, the byte returned when
        // reading palettes contains ppu open bus in the top 2 bits
        u8 p = (palettes[addr] & 0x3F) | (ppuBus & ~0x3F);

        // if greyscale is enabled, the read value is modified before being returned
        if (getGrayscale()) p &= 0xF0; // clear bottom 4 bits to achieve a "gray" color value

        // update our return value
        ret = p;
    }

    if (!readonly) ppuBus = ret;
    // return our obtained value
    return ret;
}

void PPU::ppuWrite(u16 addr, u8 data) {
    ppuAddrBus = addr;
    ppuBus = data;
    addr &= 0x3FFF; // mask address because ppu memory map only goes up to 0x3FFF
    if (addr >= 0x0000 && addr <= 0x1FFF)
        // if address is within CHR memory, write to gamepak/mapper
        cart.lock()->mapper->ppuWrite(addr, data);
    else if (addr >= 0x2000 && addr <= 0x3EFF) {
        // if address is within nametable memory, write nametable byte
        addr &= 0x0FFF; // mask address for use as array index
        u16 A = addr & 0x03FF; // nametable at $2000
        u16 B = A + 0x0400;    // nametable at $2400
        u16 C = B + 0x0400;    // nametable at $2800
        u16 D = C + 0x0400;    // nametable at $2C00

        /// @see PPU::ppuRead for table layout
        switch (cart.lock()->getMirror()) {
            default:
            case MIRROR::VERTICAL: // horizontal arrangement
                // vertical mirror means NT1 = NT3 and NT2 = NT4
                // update relevant nametable based on address
                if (addr <= 0x03FF) {
                    nametables[A] = data;
                    return;
                }
                if (addr <= 0x07FF) {
                    nametables[B] = data;
                    return;
                }
                if (addr <= 0x0BFF) {
                    nametables[A] = data;
                    return;
                }
                nametables[B] = data;
                return;
            case MIRROR::ONE_SCREEN_LO:
                // "one screen lo" (also "one screen a") means all four tables are NT1
                nametables[A] = data;
                return;
            case MIRROR::ONE_SCREEN_HI:
                // "one screen hi" (also "one screen b") means all four tables are NT2
                nametables[B] = data;
                return;
            case MIRROR::HORIZONTAL: // vertical arrangement
                // horizontal mirror means NT1 = NT2 and NT3 = NT4
                // update relevant nametable based on address
                if (addr <= 0x03FF) {
                    nametables[A] = data;
                    return;
                }
                if (addr <= 0x07FF) {
                    nametables[A] = data;
                    return;
                }
                if (addr <= 0x0BFF) {
                    nametables[B] = data;
                    return;
                }
                nametables[B] = data;
                return;
            case MIRROR::FOUR_SCREEN:
                // four screen means none of the nametables are identical
                // update relevant nametable based on address
                if (addr <= 0x03FF) {
                    nametables[A] = data;
                    return;
                }
                if (addr <= 0x07FF) {
                    nametables[B] = data;
                    return;
                }
                if (addr <= 0x0BFF) {
                    nametables[C] = data;
                    return;
                }
                nametables[D] = data;
                return;
        }
    } else if (addr <= 0x3FFF) {
        // lastly, if address is within palette memory, get palette byte
        addr &= 0x1F; // mask address for use as array index
        // these four addresses are simply mirrors of index 0
        // any other addresses lead to unique master palette indexes
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;

        // palette ram bytes are only 6 bits wide
        palettes[addr] = data & 0x3F; // masking is probably unnecessary; but better safe than sorry
    }
}

void PPU::clock() {
    // via https://www.nesdev.org/wiki/PPU_rendering#Line-by-line_timing
    if (scanline == 261) { // Pre-render scanline (-1 or 261)
        onPreRenderLine();
        // "This scanline varies in length, depending on whether an even or an odd frame is beig rendered.
        // For odd frames, the cycle at the end of the scanline is skipped (this is done internally by
        // jumping directly from (339,261) to (0,0), replacing the idle tick at the beginning of the first
        // visible scanline with the last tick of the last dummy nametable fetch)."
        // TODO: Maybe instead of skipping (340,261) we should skip (0,0)?
        if (renderingEnabled() && cycle == 339 && oddFrame) {
            cycle++;
        }
    } else if (scanline >= 0 && scanline <= 239) { // Visible scanlines (0-239)
        onVisibleLine();
    } else if (scanline == 241) { // Vertical blanking lines (241-260)
        onStartVBlankLine();
    }

    // increment cycle count
    cycle++;

    if (cycle >= 341) { // if we have done all pixels on this, scanline...
        cycle = 0; // ...reset cycle count...
        scanline++; // ...increment scanline count

        if (scanline >= 262) { // if we have done all scanlines on this frame...
            scanline = 0; // ...reset scanline count...
            frameComplete = true; // ...mark frame as complete (so console can render it)...
            oddFrame = !oddFrame; // ...alternate whether this frame # was odd or even
            // check our counters and update ppuBus based on the bit decay(s)
            for (int x = 0; x < 8; x++) {
                decayCounters[x]--;
                if (decayCounters[x] == 0) ppuBus &= ~(0x01 << x);
            }
        }
    }
}

void PPU::onPreRenderLine() {
    /* via https://www.nesdev.org/wiki/PPU_rendering#Pre-render_scanline_(-1_or_261)
     * "This is a dummy scanline, whose sole purpose is to fill the shift registers
     * with the data for the first two tiles of the next scanline. Although no pixels
     * are rendered for this scanline, the PPU still makes the same memory accesses
     * it would for a regular scanline, using whatever the current value of the PPU's
     * V register is, and for the sprite fetches, whatever data is currently in secondary
     * OAM (e.g., the results from scanline 239's sprite evaluation from the previous frame).
     * 
     * During pixels 280 through 304 of this scanline, the vertical scroll bits are
     * reloaded if rendering is enabled."
     */

    if (cycle == 1) {
        // if we're on cycle 1, we clear our status flags.
        spriteZeroHit(false);
        spritesOverflowed(false);
        inVBlank(false);
        recompNMI();
        ignoreEarlyCtrlWrites = false;
    }

    if (renderingEnabled()) {
        // background rendering process
        if ((cycle >= 1 && cycle <= 257) || (cycle >= 321 && cycle <= 337)) {
            // background pipeline operations happen during cycles 1-256 and 321-337
            backgroundPipeline();
        }

        // increment fineY only on last visible cycle; after all other rendering processes have completed
        if (cycle == 256) incrementFineY();
        // copy all horizontal bits after all rendering is complete so that we can get the correct horizontal scroll
        if (cycle == 257) copyHorizontalBits();

        if (cycle >= 257 && cycle <= 320) {
            spriteFetch(); // fetch sprite data based on OAM2 contents
        }

        // "during pixels 280 through 304 of this scanline, the vertical scroll bits are reloaded if rendering is enabled"
        if (cycle >= 280 && cycle <= 304) {
            copyVerticalBits();
        }

        // dummy nametable fetch on 337
        // dummy nametable fetch on 339
    }
}

void PPU::onVisibleLine() {
    /* via https://www.nesdev.org/wiki/PPU_rendering#Visible_scanlines_(0-239)
     * "These are the visible scanlines, which contain the graphics to be displayed
     * on the screen. This includes the rendering of both the background and the
     * sprites. During these scanlines, the PPU is busy fetching dataa, so the program
     * should NOT access PPU memory during this time, unless rendering is turned off.
     */
    // CYCLE 0
    /*
     * This is an idle cycle. The value on the PPU address bus during this cycle appears
     * to be the same CHR address that is later used to fetch the low background tile
     * byte starting at dot 5 (possibly calculated during the two unused NT fetches
     * at the end of the previous scanline).
     */
    // CYCLES 1-256
    /*
     * The data for each tile is fetched during this phase. Each memory access takes 2 PPU cycles
     * to complete, and 4 must be performed per tile:
     *      1. Nametable byte
     *      2. Attribute table byte
     *      3. Pattern table tile low
     *      4. Pattern table tile high (8 bytes above pattern table tile low address)
     * The data fetched from these accesses is placed into internal latches and then fed to the
     * appropriate shift registers when it's time to do so (every 8 cycles). Because the PPU can
     * only fetch an attribute byte every 8 cycles, each sequential string of 8 pixels is forced
     * to have the same palette attribute.
     * 
     * Sprite 0 hit acts as if the image starts at cycle 2 (which is the same cycle that the shifters
     * shift for the first time), so the sprite 0 flag will be raised at this point at the earliest.
     * Actual pixel output is delay further due to internal render pipelining, and the first pixel
     * is output during cycle 4.
     * 
     * The shifters are reloaded during ticks 9, 17, 25, ..., 257
     * 
     * NOTE: At the beginning of each scanline, the data for the first two tiles is already loaded
     * into the shift registers (and ready to be rendered), so the first tile that gets fetched
     * is Tile 3.
     * 
     * While all of this is going on, sprite evaluation for the NEXT scanline is taking place as
     * a separate process, independent to what's happening here.
     */
    // CYCLES 257-320
    /*
     * The tile data for the sprites on the NEXT scanline are fetched here. Again, each memory
     * access takes 2 PPU cycles to complete, and 4 are performed for each of the 8 sprites:
     *      1. Garbage nametable byte
     *      2. Garbage nametable byte
     *      3. Pattern table tile low
     *      4. Pattern table tile high (8 bytes above pattern table tile low address)
     * The garbage fetches occur so that the same circuitry that performs the BG tile fetches
     * could be reused for the sprite tile fetches.
     * 
     * If there are less than 8 sprites on the next scanline, then dummy fetches to tile $FF
     * occur for the left-over sprites, because of the dummy sprite data in the secondary OAM.
     * This data is then discarded, and the sprites are loaded with a transparent set of values
     * instead.
     * 
     * In addition to this, the X positions and attributes for each sprite are loaded from the
     * secondary OAM into their respective counters/latches. This happens during the second garbage
     * nametable fetch, with the attribute byte loaded during the first tick and the X coordinate
     * during the second.
     * 
     * All garbage nametable bytes except the first are the same address as the first nametable fetch
     * on the upcoming scanline. The first garbage nametable fetch is a mix due to the PPU's bus being
     * multiplexed, where the lower eight bits reflect both increments of v that happen on dot 256 but
     * the upper six bits have already been reloaded.
     */
    // CYCLES 321-336
    /*
     * This is where the first two tiles for the NEXT scanline are fetched, and loaded into the shift
     * registers. Again, each memory access takes 2 PPU cycles to complete, and 4 are performed per tile.
     *      1. Nametable byte
     *      2. Attribute table byte
     *      3. Pattern table tile low
     *      4. Pattern table tile high (8 bytes above pattern table tile low address)
     */
    // CYCLES 337-340
    /*
     * Two bytes are fetched, but the purpose for this is unknown. These fetches are 2 PPU cycles each.
     *      1. Nametable byte
     *      2. Nametable byte
     * Both of the bytes fetched here are the same nametable byte that will be fetched at the beginning
     * of the next scanline (tile 3, in other words). At least one mapper - MMC5 - is known to use this
     * string of three consecutive nametable fetches to clock a scanline counter.
     */
    if (renderingEnabled()) { // the rendering process is only possible when rendering is enabled
        // sprite rendering process
        if (cycle >= 1 && cycle <= 64) {
            initSecondaryOAM(); // clear OAM2
        } else if (cycle >= 65 && cycle <= 256) {
            spriteEval(); // evaluate sprites and fill OAM2
        } else if (cycle >= 257 && cycle <= 320) {
            spriteFetch(); // fetch sprite data based on OAM2 contents
        }

        // background rendering process
        if ((cycle >= 1 && cycle <= 257) || (cycle >= 321 && cycle <= 337)) {
            // background pipeline operations happen during cycles 1-256 and 321-337
            backgroundPipeline();
        }

        if (cycle >= 1 && cycle <= 256) {
            // render visible pixels to the screen
            renderPixel();
        }

        // increment fineY only on last visible cycle; after all other rendering processes have completed
        if (cycle == 256) incrementFineY();
        // copy all horizontal bits after all rendering is complete so that we can get the correct horizontal scroll
        if (cycle == 257) copyHorizontalBits();
    }
}

void PPU::onStartVBlankLine() {
    if (cycle == 1) {
        if (!suppressVBL)
            inVBlank(true);
        recompNMI();
        suppressVBL = false;
        suppressNMI = false;
    }
}

void PPU::incrementCoarseX() {
    if (renderingEnabled()) {
        if ((v & 0x001F) == 31) {
            v &= ~0x001F;
            v ^= 0x0400;
        } else {
            v++;
        }
    }
}

void PPU::incrementFineY() {
    if (renderingEnabled()) {
        if ((v & 0x7000) != 0x7000) {
            v += 0x1000;
        } else {
            v &= ~0x7000;
            u8 y = coarseY();
            if (y == 29) {
                y = 0;
                v ^= 0x0800;
            } else if (y == 31) {
                y = 0;
            } else {
                y++;
            }
            v = (v & ~0x03E0) | ((u16)y << 5);
        }
    }
}

void PPU::renderPixel() {
    u16 paletteAddr = 0x3F00;
    bool sprSource = false;

    u8 bgPixel = 0;
    u8 bgAttr = 0;
    u8 sprPixel = 0;
    u8 sprAttr = 0;
    u8 sprIndex = 0;
    if (renderingEnabled()) {
        getBackgroundPixel(bgPixel, bgAttr);
        sprIndex = getSpritePixel(sprPixel, sprAttr);
    }

    if (!renderBackground()) {
        bgPixel = 0;
        bgAttr = 0;
    }

    if (!renderSprites()) {
        sprPixel = 0;
        sprAttr = 0;
    }

    if ((cycle - 1) >= 0 && (cycle - 1) < 8) {
        if (!renderBackgroundLeft()) {
            bgPixel = 0;
            bgAttr = 0;
        }
        if (!renderSpritesLeft()) {
            sprPixel = 0;
            sprAttr = 0;
        }
    }

    u8 finalPixel;
    u8 finalAttr;

    if (bgPixel == 0 && sprPixel == 0) {
        finalPixel = 0;
        finalAttr = 0;
    } else if (bgPixel == 0) {
        finalPixel = sprPixel;
        finalAttr = sprAttr & 0x03;
        sprSource = true;
    } else if (sprPixel == 0) {
        finalPixel = bgPixel;
        finalAttr = bgAttr;
    } else {
        // handle sprite 0 hit
        if (sprite0HitOnThisScanline && sprIndex == 0 && cycle != 256)
            spriteZeroHit(true);

        if (spriteAboveBackground(sprAttr)) {
            finalPixel = sprPixel;
            finalAttr = sprAttr & 0x03;
            sprSource = true;
        } else {
            finalPixel = bgPixel;
            finalAttr = bgAttr;
        }
    }

    if (finalPixel != 0) paletteAddr += ((finalAttr << 2) + finalPixel);
    if (sprSource) {
        paletteAddr += 0x10;

        // if (scanline == 8)
        //     printf("Scanline: %d; cycle: %d; sprPixel: %d; sprAttr: %d; sprIndex: %d; bgPixel: %d; bgAttr: %d; finalPixel: %d; finalAttr: %d; sprSource: %s\n",
        //         scanline, cycle - 1, sprPixel, sprAttr, sprIndex, bgPixel, bgAttr, finalPixel, finalAttr, sprSource ? "true" : "false"
        //     );
    }

    u8 index = ppuRead(paletteAddr, false);

    frameBuffer[(size_t)scanline * 256 + ((size_t)cycle - 1)] = masterPalette[index & 0x3F];
}

u8 PPU::getControlData(CONTROL which) const {
    if (which == CONTROL::NAMETABLE_BASE) return PPUCTRL & 0x03;
    else return ((PPUCTRL >> (u8)which) & 0x01);
}

void PPU::setStatusData(STATUS which, bool v) {
    if (v) {
        PPUSTATUS |= (0x01 << (u8)which);
    } else {
        PPUSTATUS &= (~(0x01 << (u8)which));
    }
}

u32 PPU::applyEmphasis(u32 color) const {
    u32 r = (color >> 0) & 0xFF;
    u32 g = (color >> 8) & 0xFF;
    u32 b = (color >> 16) & 0xFF;

    if (this->getGrayscale())
        r = g = b = (u32)std::floor((r + g + b) / 3);

    bool eR = this->getMaskData(MASK::EMPHASIZE_RED);
    bool eG = this->getMaskData(MASK::EMPHASIZE_GREEN);
    bool eB = this->getMaskData(MASK::EMPHASIZE_BLUE);

    if (eR || eG || eB) {
        bool eA = eR && eG && eB;

        if (eA || !eR)
            r = (u32)std::floor(r * 0.75);
        if (eA || !eG)
            g = (u32)std::floor(g * 0.75);
        if (eA || !eB)
            b = (u32)std::floor(b * 0.75);
    }

    return 0xFF000000 | (r << 0) | (g << 8) | (b << 16);
}

void PPU::updateCounters(u8 bits) {
    for (int x = 0; x < 8; x++) {
        if (((bits >> x) & 0x01) > 0) decayCounters[x] = 20;
    }
}

void PPU::bitDecay() {
    for (int x = 0; x < 8; x++) {
        if (decayCounters[x] == 0) {
            u8 mask = 1 << x;
            ppuBus &= ~mask;
        }
    }
}

u8 PPU::readSpriteAttr(SPRITE_ATTR which, u8 attr) {
    switch (which) {
        case PALETTE:
            return (attr & 0x03);
        case PRIORITY:
            return ((attr >> 5) & 0x01);
        case XFLIP:
            return ((attr >> 6) & 0x01);
        case YFLIP:
            return ((attr >> 7) & 0x01);
        default:
            return 0x00;
    }
}

void PPU::recompNMI() {
    bool prevNMI = nmiOutput;
    nmiOutput = inVBlank() && getNMIEnabled();
    if (!prevNMI && nmiOutput && !suppressNMI)
        nmiRequested = true;
}