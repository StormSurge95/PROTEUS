#include "./GbaBus.h"

using namespace NS_GBA;

bool GbaBus::requestFetch(
    GbaBusMaster master,
    u32 address,
    GbaAccessWidth width,
    bool sequentialAccess
) {
    return beginTransaction(
        AccessKind::FETCH,
        master,
        address,
        width,
        sequentialAccess,
        0
    );
}

bool GbaBus::requestRead(
    GbaBusMaster master,
    u32 address,
    GbaAccessWidth width,
    bool sequentialAccess
) {
    return beginTransaction(
        AccessKind::READ,
        master,
        address,
        width,
        sequentialAccess,
        0
    );
}

bool GbaBus::requestWrite(
    GbaBusMaster master,
    u32 address,
    GbaAccessWidth width,
    bool sequentialAccess,
    u32 data
) {
    return beginTransaction(
        AccessKind::WRITE,
        master,
        address,
        width,
        sequentialAccess,
        data
    );
}

void GbaBus::clock() {
    // TODO
}

bool GbaBus::responseReady(GbaBusMaster master) const {
    return transaction.state == TransferState::COMPLETE &&
        transaction.master == master;
}

bool GbaBus::takeResponse(GbaBusMaster master, u32& response) {
    if (!responseReady(master)) return false;

    response = transaction.data;
    transaction.reset();
    return true;
}

u32 GbaBus::peek(u32 address, GbaAccessWidth width) {
    const DecodedAddress addr = decodeAddress(address);

    // TODO: find a way to implement the multi-byte reads
    // that doesn't allow us to advance beyond the bounds
    // of the relevant memory container
    u8* mem = nullptr;

    switch (addr.region) {
        case MemoryRegion::BIOS:
            mem = &bios[addr.offset];
            break;
        case MemoryRegion::EWRAM:
            mem = &ewram[addr.offset];
            break;
        case MemoryRegion::IWRAM:
            mem = &iwram[addr.offset];
            break;
        case MemoryRegion::UNMAPPED:
        default:
            return 0;
    }

    u32 ret = 0;

    switch (width) {
        case GbaAccessWidth::WORD:
            ret |= (
                (static_cast<u32>(mem[3]) << 24) |
                (static_cast<u32>(mem[2]) << 16)
            );
        case GbaAccessWidth::HALFWORD:
            ret |= (static_cast<u32>(mem[1]) << 8);
        case GbaAccessWidth::BYTE:
            ret |= static_cast<u32>(mem[0]);
        default:
        return ret;
    }
}

void GbaBus::Transaction::reset() {
    state = TransferState::IDLE;
    kind = AccessKind::READ;

    master = GbaBusMaster::ARM7;
    width = GbaAccessWidth::BYTE;

    address = 0;
    data = 0;

    sequentialAccess = false;

    cyclesRemaining = 0;

    nativeAccessIndex = 0;
    nativeAccessCount = 1;
}

bool GbaBus::beginTransaction(
    AccessKind kind,
    GbaBusMaster master,
    u32 address,
    GbaAccessWidth width,
    bool sequentialAccess,
    u32 data
) {
    if (!transaction.isIdle()) return false;

    transaction.state = TransferState::ACTIVE;
    transaction.kind = kind;

    transaction.master = master;
    transaction.width = width;

    transaction.address = address;
    transaction.data = data;

    transaction.sequentialAccess = sequentialAccess;
    
    transaction.nativeAccessCount = 0;
    transaction.nativeAccessIndex = 1;

    const DecodedAddress addr = decodeAddress(address);

    switch (addr.region) {
        default:
        case MemoryRegion::BIOS:
        case MemoryRegion::IWRAM:
        case MemoryRegion::IO:
        case MemoryRegion::UNMAPPED:
            transaction.cyclesRemaining = 1;
            break;
        case MemoryRegion::OAM:
            // TODO: plus 1 cycle if GBA accesses video memory at the same time
            transaction.cyclesRemaining = 1;
            break;
        case MemoryRegion::EWRAM:
            // TODO: default waitstate settings (?)
            if (width == GbaAccessWidth::WORD)
                transaction.cyclesRemaining = 6;
            else transaction.cyclesRemaining = 3;
            break;
        case MemoryRegion::PALETTE:
        case MemoryRegion::VRAM:
            // TODO: plus 1 cycle if GBA accesses video memory at the same time
            if (width == GbaAccessWidth::WORD)
                transaction.cyclesRemaining = 2;
            else transaction.cyclesRemaining = 1;
            break;
        case MemoryRegion::GAMEPAK_WS0:
        case MemoryRegion::GAMEPAK_WS1:
        case MemoryRegion::GAMEPAK_WS2:
            // TODO: default waitstate settings (?)
            // TODO: separate timings for sequential, and non-sequential accesses
            if (width == GbaAccessWidth::WORD)
                transaction.cyclesRemaining = 8;
            else transaction.cyclesRemaining = 5;
            break;
        case MemoryRegion::GAMEPAK_RAM:
            // TODO: default waitstate settings (?)
            transaction.cyclesRemaining = 5;
            break;
    }

    beginNativeAccess();

    return true;
}

void GbaBus::clockTransaction() {
    // TODO
}

void GbaBus::beginNativeAccess() {
    // TODO
}

void GbaBus::completeNativeAccess() {
    // TODO
}

u32 GbaBus::currentNativeAddress() const {
    // TODO
    return 0;
}

bool GbaBus::currentNativeAddressIsSequential() const {
    // TODO
    return false;
}

u32 GbaBus::calculateAccessCycles(
    u32 address,
    GbaAccessWidth width,
    bool sequentialAccess,
    AccessKind kind
) const {
    // TODO
    return 0;
}

u32 GbaBus::readNative(
    u32 address,
    GbaAccessWidth width,
    bool readonly
) {
    if (!readonly) {
        // TODO: implement various read side-effects
    }
    return peek(address, width);
}

void GbaBus::writeNative(
    u32 address,
    GbaAccessWidth width,
    u32 data
) {
    const DecodedAddress addr = decodeAddress(address);

    u8* mem = nullptr;

    switch (addr.region) {
        case MemoryRegion::EWRAM:
            mem = &ewram[addr.offset];
            break;
        case MemoryRegion::IWRAM:
            mem = &iwram[addr.offset];
            break;
        default:
            return;
    }

    switch (width) {
        case GbaAccessWidth::WORD:
            mem[3] = static_cast<u8>(data >> 24);
            mem[2] = static_cast<u8>(data >> 16);
        case GbaAccessWidth::HALFWORD:
            mem[1] = static_cast<u8>(data >> 8);
        case GbaAccessWidth::BYTE:
            mem[0] = static_cast<u8>(data);
        default:
            return;
    }
}

DecodedAddress GbaBus::decodeAddress(u32 address) const {
    // BIOS
    if (address >= 0x0000'0000 && address <= 0x0000'3FFF) {
        return {
            .region = MemoryRegion::BIOS,
            .offset = address
        };
    }
    // EWRAM
    else if (address >= 0x0200'0000 && address <= 0x0203'FFFF) {
        return {
            .region = MemoryRegion::EWRAM,
            .offset = address - 0x0200'0000
        };
    }
    // IWRAM
    else if (address >= 0x0300'0000 && address <= 0x0300'7FFF) {
        return {
            .region = MemoryRegion::IWRAM,
            .offset = address - 0x0300'0000
        };
    }
    // IO
    else if (address >= 0x0400'0000 && address <= 0x04FF'FFFF) {
        // TODO: verify end of IO memory region
        return {
            .region = MemoryRegion::IO,
            .offset = address - 0x0400'0000
        };
    }
    // PALETTE
    else if (address >= 0x0500'0000 && address <= 0x0500'03FF) {
        return {
            .region = MemoryRegion::PALETTE,
            .offset = address - 0x0500'0000
        };
    }
    // VRAM
    else if (address >= 0x0600'0000 && address <= 0x0601'7FFF) {
        return {
            .region = MemoryRegion::VRAM,
            .offset = address - 0x0600'0000
        };
    }
    // OAM
    else if (address >= 0x0700'0000 && address <= 0x0700'03FF) {
        return {
            .region = MemoryRegion::OAM,
            .offset = address - 0x0700'0000
        };
    }
    // Gamepak Wait State 0
    else if (address >= 0x0800'0000 && address <= 0x09FF'FFFF) {
        return {
            .region = MemoryRegion::GAMEPAK_WS0,
            .offset = address - 0x0800'0000
        };
    }
    // Gamepak Wait State 1
    else if (address >= 0x0A00'0000 && address <= 0x0BFF'FFFF) {
        return {
            .region = MemoryRegion::GAMEPAK_WS1,
            .offset = address - 0x0A00'0000
        };
    }
    // Gamepak Wait State 2
    else if (address >= 0x0C00'0000 && address <= 0x0DFF'FFFF) {
        return {
            .region = MemoryRegion::GAMEPAK_WS2,
            .offset = address - 0x0C00'0000
        };
    }
    // Gamepak RAM
    else if (address >= 0x0E00'0000 && address <= 0x0E00'FFFF) {
        return {
            .region = MemoryRegion::GAMEPAK_RAM,
            .offset = address - 0x0E00'0000
        };
    }
    // UNMAPPED
    else {
        return {
            .region = MemoryRegion::UNMAPPED,
            .offset = 0
        };
    }
}

u32 GbaBus::alignAddress(const u32 address, const GbaAccessWidth width) {
    switch (width) {
        case GbaAccessWidth::WORD:
            return (address & ~3u);
        case GbaAccessWidth::HALFWORD:
            return (address & ~1u);
        default:
        case GbaAccessWidth::BYTE:
            return address;
    }
}