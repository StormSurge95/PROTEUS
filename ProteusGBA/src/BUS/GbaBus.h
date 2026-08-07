#pragma once

#include "../shared/GbaPCH.h"

namespace NS_GBA {
    class GbaBus {
        public:
            GbaBus() = default;
            ~GbaBus() = default;

            /**
             * @brief Ask the bus to begin a `fetch` request for opcodes/instructions
             * @param master The device making the request
             * @param address The address to be read from
             * @param width The data-width being requested
             * @param sequentialAccess `true` if SEQUENTIAL
             * @return `true` if the request is accepted; `false` otherwise
             */
            bool requestFetch(
                GbaBusMaster master,
                u32 address,
                GbaAccessWidth width,
                bool sequentialAccess
            );

            /**
             * @brief Ask the bus to begin a `read` transaction for data
             * @param master The device requesting the read
             * @param address The address to be read from
             * @param width The data-width being requested
             * @param sequentialAccess `true` if SEQUENTIAL
             * @return `true` if the request is accepted; `false` otherwise
             */
            bool requestRead(
                GbaBusMaster master,
                u32 address,
                GbaAccessWidth width,
                bool sequentialAccess
            );

            /**
             * @brief Ask the bus to begin a `write` transaction
             * @param master The device requesting the write
             * @param address The address to be written to
             * @param width The data-width being written
             * @param sequentialAccess `true` if SEQUENTIAL
             * @param data The data being written
             * @return `true` if the request is accepted; `false` otherwise
             */
            bool requestWrite(
                GbaBusMaster master,
                u32 address,
                GbaAccessWidth width,
                bool sequentialAccess,
                u32 data
            );

            /**
             * @brief Advance the active transaction and arbitration state
             * @details Advances all timing by exactly one GBA master cycle.
             *          Upon reaching a completion state, the bus invokes
             *          the mapped device's native-width `read()` or `write()`.
             */
            void clock();

            /**
             * @brief Ask the bus if a transaction has been completed.
             * @param master The device requesting the status update.
             * @return `true` if the transaction is complete; `false` otherwise
             */
            bool responseReady(GbaBusMaster master) const;

            /**
             * @brief Retrieve and consume the completed response.
             * @param master The device retrieving the response.
             * @return The current value of the address accessed.
             * @note The return value is generally ignored by `write` requests.
             */
            bool takeResponse(GbaBusMaster master, u32& response);

            /**
             * @brief Perform an immediate, untimed, side-effect-free data read.
             * @param address The address to be read from
             * @param width The data-width being read
             * @return The data that was read
             */
            u32 peek(u32 address, GbaAccessWidth width);
        private:
            enum class AccessKind {
                FETCH,
                READ,
                WRITE
            };
            enum class TransferState : u8 {
                IDLE,
                ACTIVE,
                COMPLETE
            };
            struct Transaction {
                TransferState state = TransferState::IDLE;
                AccessKind kind = AccessKind::READ;

                GbaBusMaster master = GbaBusMaster::ARM7;
                GbaAccessWidth width = GbaAccessWidth::BYTE;

                u32 address = 0;
                u32 data = 0;

                bool sequentialAccess = false;

                u32 cyclesRemaining = 0;

                u8 nativeAccessIndex = 0;
                u8 nativeAccessCount = 1;

                void reset();
                bool isIdle() const { return state == TransferState::IDLE; }
            };
            Transaction transaction{};

            // BIOS STORAGE
            array<u8, 0x4000> bios = {};
            // EWRAM STORAGE
            array<u8, 0x0004'0000> ewram = {};
            // IWRAM STORAGE
            array<u8, 0x8000> iwram = {};

            /**
             * @brief Verify current bus state and start a new transaction (if possible)
             * @param kind The kind of memory access
             * @param master The device to start a transaction for
             * @param address The address of the transaction
             * @param width The data-width of the memory access
             * @param sequentialAccess Whether this particular access is sequential
             * @param data The data used for the access (only effects `write` operations)
             * @return `true` if transaction was accepted; `false` otherwise
             */
            bool beginTransaction(
                AccessKind kind,
                GbaBusMaster master,
                u32 address,
                GbaAccessWidth width,
                bool sequentialAccess,
                u32 data
            );

            void clockTransaction();

            void beginNativeAccess();
            void completeNativeAccess();

            u32 currentNativeAddress() const;
            bool currentNativeAddressIsSequential() const;

            u32 calculateAccessCycles(
                u32 address,
                GbaAccessWidth width,
                bool sequentialAccess,
                AccessKind kind
            ) const;

            u32 readNative(
                u32 address,
                GbaAccessWidth width,
                bool readonly = false
            );

            void writeNative(
                u32 address,
                GbaAccessWidth width,
                u32 data
            );

            DecodedAddress decodeAddress(u32 address) const;

            u32 alignAddress(const u32 address, const GbaAccessWidth width);
    };
}