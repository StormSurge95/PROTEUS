#pragma once

#include "../FrontendPCH.h"
#include "ConsoleSession.h"

namespace NS_Proteus {
    /// @brief Stable identifier for managed console sessions.
    using SessionID = u32;

    /// @brief Per-frame scheduling policy for managed sessions.
    enum class ScheduleMode {
        /// @brief Only the active session is advanced each frame.
        SINGLE,
        
        /// @brief All running sessions are advanced each frame.
        MULTI
    };

    /// @brief Read-only snapshot describing one managed session.
    struct SessionInfo {
        /// @brief Stable session identifier.
        SessionID id = 0;

        /// @brief Console type currently bound to this session.
        ConsoleID console = ConsoleID::NONE;

        /// @brief Current lifecycle state of the session.
        ConsoleSessionState state = ConsoleSessionState::EMPTY;

        /// @brief Display name of loaded ROM, if any.
        string romName = "";

        /// @brief Path of loaded ROM, if any.
        path romPath = {};

        /// @brief True if this is the active/focused session.
        bool active = false;
    };

    /// @brief Runtime configuration for session management.
    struct SessionManagerConfig {
        /// @brief Session clocking policy used by `ClockSessions()`.
        ScheduleMode scheduleMode = ScheduleMode::SINGLE;
        
        /// @brief If true, only play audio routed from the active session.
        bool audioActiveOnly = true;

        /// @brief If true, gameplay input is routed only to the active session.
        bool inputActiveOnly = true;
    };

    /**
     * @class SessionManager
     * @brief Coordinates multiple ConsoleSession instances.
     * @details Owns session slots, active-session focus, and frame scheduling.
     *          All session creation/destruction is centralized here.
     */
    class SessionManager {
        private:
            /**
             * @brief Internal owned slot for one session.
             * @details Contains mutable runtime ownership and bookkeeping.
             */
            struct Session {
                /// @brief Stable id for this slot.
                SessionID id = 0;
                
                /// @brief Console type bound to this slot.
                ConsoleID console = ConsoleID::NONE;

                /// @brief Owned session instance.
                uptr<ConsoleSession> session;

                /// @brief Getter for ROM display name from `session` 
                string romName() const { return session->GetRomName(); }

                /// @brief Getter for ROM path from `session`
                path romPath() const { return session->GetRomPath(); }
            };

            /// @brief True when `Initialize()` has succeeded.
            bool initialized = false;

            /// @brief Monotonic id generator for new sessions.
            SessionID nextSessionID = 1;

            /// @brief Currently active/focused session id (0 = none).
            SessionID activeSessionID = 0;

            /// @brief Runtime configuration.
            SessionManagerConfig config{};

            /// @brief Non-owning logger pointer.
            Logger* logger = nullptr;

            /// @brief Human-readable error message for manager-level failures.
            string lastError = "";

            /// @brief Stable presentation order of session ids.
            vector<SessionID> order;

            /// @brief Owned session slots keyed by session id.
            unordered_map<SessionID, Session> sessions;

            /**
             * @brief Find mutable internal slot by id.
             * @param id Session id.
             * @return Slot pointer or nullptr.
             */
            Session* FindSlot(SessionID id);

            /**
             * @brief Find immutable internal slot by id.
             * @param id Session id.
             * @return Slot pointer or nullptr.
             */
            const Session* FindSlot(SessionID id) const;

            /// @brief Destroy all sessions and clear storage.
            void ClearAllSessions();
            
            /**
             * @brief Ensure active session remains valid after mutations.
             * @details Promotes first available slot to active if current active is invalid.
             */
            void PromoteSession();
        public:
            /**
             * @brief Construct orchestrator with logger dependency.
             * @param logger Non-owning logger pointer used for diagnostics/events.
             */
            explicit SessionManager(Logger* logger);

            /// @brief Destructor. Calls Shutdown() for cleanup
            ~SessionManager() { Shutdown(); }

            // Singleton class; remove all copy-constructors
            SessionManager(const SessionManager&) = delete;
            SessionManager& operator=(const SessionManager&) = delete;
            SessionManager(SessionManager&&) = delete;
            SessionManager& operator=(SessionManager&&) = delete;

            /**
             * @brief Initialize management runtime.
             * @param cfg Initial management config.
             * @return true on success; false otherwise.
             * @note Upon any failure, will update `lastError`.
             */
            bool Initialize(const SessionManagerConfig& cfg = {});

            /// @brief Shutdown Manager and destroy all sessions.
            void Shutdown();

            /**
             * @brief Create a new session for a console.
             * @param [in] console Console type to instantiate.
             * @param [out] outID Receives assigned SessionID on success.
             * @return SessionResult from underlying session creation flow.
             */
            SessionResult CreateSession(ConsoleID console, SessionID& outID);

            /**
             * @brief Destroy a session and release its resources.
             * @param id Session to destroy.
             * @return SessionResult success/failure details.
             */
            SessionResult DestroySession(SessionID id);
            
            /**
             * @brief Load ROM into a specific session.
             * @param id Target session id.
             * @param romPath ROM file path.
             * @param romName Display name for UI/logging.
             * @return SessionResult success/failure details.
             */
            SessionResult LoadROM(SessionID id, const path& romPath, const string& romName);

            /**
             * @brief Start or resume a specific session.
             * @param id Target session id.
             * @return SessionResult success/failure details.
             */
            SessionResult Start(SessionID id);

            /**
             * @brief Pause a specific session.
             * @param id Target session id.
             * @return SessionResult success/failure details.
             */
            SessionResult Pause(SessionID id);

            /**
             * @brief Reset ROM state for a specific session.
             * @param id Target session id.
             * @return SessionResult success/failure details.
             */
            SessionResult Reset(SessionID id);

            /**
             * @brief Set active/focused session.
             * @param id Session to mark active.
             * @return true if id exists and active session updated.
             */
            bool SetActiveSession(SessionID id);

            /// @brief Get current active session id. 
            SessionID GetActiveSessionID() const { return activeSessionID; }

            /// @brief True if an active session is currently selected.
            bool HasActiveSession() const { return activeSessionID != 0; }

            /**
             * @brief Check if a session id exists.
             * @param id Session id to query.
             */
            bool HasSession(SessionID id) const;

            /**
             * @brief Get mutable session pointer by id.
             * @param id Session id.
             * @return Non-owning pointer, or nullptr if missing.
             */
            ConsoleSession* GetSession(SessionID id);

            /**
             * @brief Get immutable session pointer by id.
             * @param id Session id.
             * @return Non-owning pointer, or nullptr if missing.
             */
            const ConsoleSession* GetSession(SessionID id) const;

            /**
             * @brief Get mutable pointer to active session.
             * @return Non-owning pointer, or nullptr if non active.
             */
            ConsoleSession* GetActiveSession();

            /**
             * @brief Get immutable pointer to active session.
             * @return Non-owning pointer, or nullptr if non active.
             */
            const ConsoleSession* GetActiveSession() const;
            
            /**
             * @brief Build snapshot descriptors for all sessions.
             * @return Ordered list of session descriptors.
             */
            vector<SessionInfo> ListSessions() const;

            /// @brief Number of tracked session slots.
            size_t SessionCount() const { return sessions.size(); }

            /**
             * @brief Per-frame scheduler entrypoint.
             * @details Advances session execution according to ScheduleMode.
             */
            void ClockSessions();

            /**
             * @brief Get active session console pointer.
             * @return Shared pointer to active console, or nullptr.
             */
            sptr<IConsole> GetActiveConsole() const;

            /**
             * @brief Get active session debugger pointer.
             * @return Shared pointer to active debugger, or nullptr.
             */
            sptr<IDebugger> GetActiveDebugger() const;

            /**
             * @brief Set frame schedule mode.
             * @param mode New schedule mode.
             */
            void SetScheduleMode(ScheduleMode mode) { config.scheduleMode = mode; }
            
            /// @brief Get current schedule mode.
            ScheduleMode GetScheduleMode() const { return config.scheduleMode; }

            /// @brief Get last managment-layer error message.
            const string& GetLastError() const { return lastError; }

            /**
             * TODO: FINISH LINKAGE STUFF
             */
    };
}