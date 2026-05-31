#pragma once

#include "../FrontendPCH.h"
#include "SessionManager.h"

namespace NS_Proteus {
    /// @brief Stable identifier for one active link connection
    using LinkID = u32;

    /// @brief Transport/bridge status codes
    enum class LinkStatus {
        DISCONNECTED,
        CONNECTED,
        ERROR
    };

    /**
     * @brief Logical link mode
     * @details Extend as addition console families are implemented.
     */
    enum class LinkMode {
        NONE,
        GBA_CABLE,
        GB_SERIAL
    };

    /**
     * @brief Lightweight payload envelope for inter-session transfer.
     * @details Keep generic at management layer; concrete cores interpret bytes.
     */
    struct LinkPacket {
        SessionID from = 0;
        SessionID to = 0;
        vector<u8> data{};
        u64 tick = 0;
    };

    /// @brief Public snapshot of a link connection
    struct LinkInfo {
        const LinkID id;
        const SessionID a;
        const SessionID b;
        const LinkMode mode;
        const LinkStatus status;
        const string lastError;

        LinkInfo(LinkID i, SessionID s1, SessionID s2, LinkMode m, LinkStatus s, string l) :
            id(i), a(s1), b(s2), mode(m), status(s), lastError(l) {}
    };

    /**
     * @brief Bridge interface for optional inter-session communication.
     * @note Implementations can be pure in-memory initially; transport can evolve later.
     */
    class ILinkBridge {
        public:
            virtual ~ILinkBridge() = default;

            /**
             * @brief Connect two session endpoints for a specific mode.
             * @param id Link identifier assigned by manager.
             * @param a Endpoint session A.
             * @param b Endpoint session B.
             * @param mode Link mode to establish.
             * @return true on success
             */
            virtual bool Connect(LinkID id, SessionID a, SessionID b, LinkMode mode) = 0;

            /**
             * @brief Disconnect an existing link.
             * @param id Link identifier.
             * @return true on success.
             */
            virtual bool Disconnect(LinkID id) = 0;

            /**
             * @brief Queue packet for transfer.
             * @param id Link identifier.
             * @param packet Packet envelope.
             * @return true on success.
             */
            virtual bool PushPacket(LinkID id, const LinkPacket& packet) = 0;

            /**
             * @brief Advance bridge internals one clock tick.
             * @note Called from SessionManager frame loop.
             */
            virtual void Pump() = 0;

            /// @brief Read current status of a link.
            virtual LinkStatus GetStatus(LinkID id) const = 0;

            /// @brief Human-readable last bridge error.
            virtual const string& GetLastError() const = 0;
    };

    /// @brief Internal storage for one managed link connection.
    struct LinkConnection {
        LinkID id = 0;
        SessionID a = 0;
        SessionID b = 0;
        LinkMode mode = LinkMode::NONE;
        LinkStatus status = LinkStatus::DISCONNECTED;
        string lastError = "";

        const LinkInfo GetInfo() const {
            return {
                id, a, b, mode, status, lastError
            };
        }
    };
}